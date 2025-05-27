#!/usr/bin/env python3
"""
EmEngine C++/C# Binding Generator

This script automatically generates C# P/Invoke declarations from C++ extern "C" functions.
It parses C++ header and source files to find exported functions and creates corresponding
C# interface code with proper type marshaling.

Usage:
    python generate_bindings.py [--output-dir OUTPUT_DIR] [--namespace NAMESPACE]
"""

import os
import re
import argparse
import sys
from typing import List, Dict, Tuple, Optional
from dataclasses import dataclass
from pathlib import Path


@dataclass
class FunctionParam:
    name: str
    cpp_type: str
    csharp_type: str
    marshaling_attr: str = ""


@dataclass 
class ExternFunction:
    name: str
    return_type_cpp: str
    return_type_csharp: str
    params: List[FunctionParam]
    file_path: str
    line_number: int


class TypeMapper:
    """Maps C++ types to C# types with appropriate marshaling attributes."""
    
    def __init__(self):
        self.type_mappings = {
            'void': 'void',
            'bool': 'bool',
            'int': 'int', 
            'unsigned int': 'int',  # Match existing pattern
            'float': 'float',
            'double': 'double',
            'const char*': ('string', ''),  # No marshaling attr needed for simple strings
            'char*': ('string', ''),
        }
        
        # Array types need special handling
        self.array_pattern = re.compile(r'(\w+)\s*\[\s*(\d*)\s*\]')
        
    def map_type(self, cpp_type: str) -> Tuple[str, str]:
        """Map C++ type to C# type and marshaling attribute."""
        cpp_type = cpp_type.strip()
        
        # Handle arrays
        array_match = self.array_pattern.match(cpp_type)
        if array_match:
            base_type = array_match.group(1)
            size = array_match.group(2)
            if base_type == 'float':
                return 'float[]', '[In]'
            elif base_type == 'int':
                return 'int[]', '[In]'
                
        # Handle pointer types (special case for GetPressed)
        if cpp_type.endswith('*') and not cpp_type.startswith('const char'):
            if 'int' in cpp_type:
                return 'IntPtr', 'out'  # Match existing GetPressed pattern
            
        # Direct mappings
        if cpp_type in self.type_mappings:
            mapping = self.type_mappings[cpp_type]
            if isinstance(mapping, tuple):
                return mapping
            else:
                return mapping, ''
                
        # Default fallback
        return 'IntPtr', ''


class CppParser:
    """Parses C++ files to extract extern "C" function declarations."""
    
    def __init__(self):
        self.type_mapper = TypeMapper()
        
    def parse_files(self, source_dirs: List[str]) -> List[ExternFunction]:
        """Parse all C++ files in given directories for extern C functions."""
        functions = []
        
        for source_dir in source_dirs:
            for root, dirs, files in os.walk(source_dir):
                for file in files:
                    if file.endswith(('.cpp', '.hpp', '.h')):
                        file_path = os.path.join(root, file)
                        functions.extend(self._parse_file(file_path))
        
        # Deduplicate functions by name (keep first occurrence)
        seen_names = set()
        unique_functions = []
        for func in functions:
            if func.name not in seen_names:
                seen_names.add(func.name)
                unique_functions.append(func)
                        
        return unique_functions
        
    def _parse_file(self, file_path: str) -> List[ExternFunction]:
        """Parse a single C++ file for extern C functions."""
        functions = []
        
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
        except UnicodeDecodeError:
            print(f"Warning: Could not read {file_path} (encoding issue)")
            return functions
            
        # Find extern "C" blocks - handle nested braces
        extern_c_pattern = re.compile(r'extern\s+"C"\s*\{(.*)\};?', re.DOTALL)
        
        for match in extern_c_pattern.finditer(content):
            extern_block = match.group(1)
            block_start = match.start()
            
            # Parse functions within the extern C block
            func_functions = self._parse_functions_in_block(extern_block, file_path, content, block_start)
            functions.extend(func_functions)
            
        # Also look for standalone extern "C" function declarations
        standalone_pattern = re.compile(r'extern\s+"C"\s+([^{;]+;)', re.MULTILINE)
        for match in standalone_pattern.finditer(content):
            func_decl = match.group(1).strip()
            line_num = content[:match.start()].count('\n') + 1
            
            parsed_func = self._parse_function_declaration(func_decl, file_path, line_num)
            if parsed_func:
                functions.append(parsed_func)
                
        return functions
        
    def _parse_functions_in_block(self, block_content: str, file_path: str, full_content: str, block_start: int) -> List[ExternFunction]:
        """Parse function declarations within an extern C block."""
        functions = []
        
        # Remove preprocessor directives and comments first
        cleaned_content = re.sub(r'#.*\n', '\n', block_content)
        cleaned_content = re.sub(r'//.*\n', '\n', cleaned_content)
        
        # Handle both declarations and definitions
        # For declarations: return_type function_name(params);
        decl_pattern = re.compile(r'(\w+(?:\s*\*)?)\s+(\w+)\s*\(\s*([^)]*?)\s*\)\s*;', re.MULTILINE | re.DOTALL)
        
        # For definitions: return_type function_name(params) { ... }
        def_pattern = re.compile(r'(\w+(?:\s*\*)?)\s+(\w+)\s*\(\s*([^)]*?)\s*\)\s*\{', re.MULTILINE | re.DOTALL)
        
        # Process declarations
        for match in decl_pattern.finditer(cleaned_content):
            return_type = match.group(1).strip()
            func_name = match.group(2).strip()
            params_str = match.group(3).strip()
            
            match_pos_in_full = block_start + match.start()
            line_num = full_content[:match_pos_in_full].count('\n') + 1
            
            func = self._create_extern_function(return_type, func_name, params_str, file_path, line_num)
            if func:
                functions.append(func)
        
        # Process definitions (extract signature before the opening brace)
        for match in def_pattern.finditer(cleaned_content):
            return_type = match.group(1).strip()
            func_name = match.group(2).strip()
            params_str = match.group(3).strip()
            
            match_pos_in_full = block_start + match.start()
            line_num = full_content[:match_pos_in_full].count('\n') + 1
            
            func = self._create_extern_function(return_type, func_name, params_str, file_path, line_num)
            if func:
                functions.append(func)
            
        return functions
        
    def _create_extern_function(self, return_type: str, func_name: str, params_str: str, file_path: str, line_num: int) -> Optional[ExternFunction]:
        """Create an ExternFunction object from parsed components."""
        # Parse parameters
        params = self._parse_parameters(params_str)
        
        # Map types
        return_type_cs, _ = self.type_mapper.map_type(return_type)
        
        mapped_params = []
        for param_name, param_type in params:
            cs_type, marshaling = self.type_mapper.map_type(param_type)
            mapped_params.append(FunctionParam(param_name, param_type, cs_type, marshaling))
        
        return ExternFunction(
            name=func_name,
            return_type_cpp=return_type,
            return_type_csharp=return_type_cs,
            params=mapped_params,
            file_path=file_path,
            line_number=line_num
        )
        
    def _parse_function_declaration(self, decl: str, file_path: str, line_num: int) -> Optional[ExternFunction]:
        """Parse a single function declaration."""
        # Remove trailing semicolon
        decl = decl.rstrip(';').strip()
        
        # Pattern to match function declaration
        func_pattern = re.compile(r'(\w+(?:\s*\*)?)\s+(\w+)\s*\(([^)]*)\)')
        match = func_pattern.search(decl)
        
        if not match:
            return None
            
        return_type = match.group(1).strip()
        func_name = match.group(2).strip()
        params_str = match.group(3).strip()
        
        # Parse parameters
        params = self._parse_parameters(params_str)
        
        # Map types
        return_type_cs, _ = self.type_mapper.map_type(return_type)
        
        mapped_params = []
        for param_name, param_type in params:
            cs_type, marshaling = self.type_mapper.map_type(param_type)
            mapped_params.append(FunctionParam(param_name, param_type, cs_type, marshaling))
        
        return ExternFunction(
            name=func_name,
            return_type_cpp=return_type,
            return_type_csharp=return_type_cs,
            params=mapped_params,
            file_path=file_path,
            line_number=line_num
        )
        
    def _parse_parameters(self, params_str: str) -> List[Tuple[str, str]]:
        """Parse function parameters string into list of (name, type) tuples."""
        if not params_str or params_str.strip() == 'void':
            return []
            
        params = []
        param_parts = [p.strip() for p in params_str.split(',')]
        
        for param in param_parts:
            if not param:
                continue
                
            # Handle array parameters like "float pos[3]"
            array_match = re.match(r'(\w+(?:\s*\*)?)\s+(\w+)\s*\[\s*\d*\s*\]', param)
            if array_match:
                param_type = array_match.group(1) + '[]'
                param_name = array_match.group(2)
                params.append((param_name, param_type))
                continue
                
            # Handle regular parameters
            parts = param.split()
            if len(parts) >= 2:
                # Join all but last part as type, last part as name
                param_type = ' '.join(parts[:-1])
                param_name = parts[-1]
                
                # Handle pointer parameters
                if param_name.startswith('*'):
                    param_type += '*'
                    param_name = param_name[1:]
                    
                params.append((param_name, param_type))
            elif len(parts) == 1:
                # Type only, generate parameter name
                param_type = parts[0]
                param_name = f"param{len(params)}"
                params.append((param_name, param_type))
                
        return params


class CSharpGenerator:
    """Generates C# P/Invoke code from parsed C++ functions."""
    
    def __init__(self, namespace: str = "EmEngine.Bindings"):
        self.namespace = namespace
        
    def generate(self, functions: List[ExternFunction]) -> str:
        """Generate complete C# file with P/Invoke declarations."""
        # Sort functions by name for consistent output
        functions = sorted(functions, key=lambda f: f.name)
        
        code_lines = [
            "// <auto-generated>",
            "// This file was automatically generated by generate_bindings.py", 
            "// Do not modify this file directly - your changes will be overwritten",
            "// </auto-generated>",
            "",
            "using System;",
            "using System.Runtime.InteropServices;",
            "using Input;",
            "",
            f"namespace {self.namespace}",
            "{",
            "    public static class EngineApi",
            "    {",
            ""
        ]
        
        for func in functions:
            code_lines.extend(self._generate_function(func))
            code_lines.append("")
            
        code_lines.extend([
            "    }",
            "}"
        ])
        
        return '\n'.join(code_lines)
        
    def _generate_function(self, func: ExternFunction) -> List[str]:
        """Generate P/Invoke declaration for a single function."""
        lines = []
        
        # Add source comment
        lines.append(f"        // Source: {os.path.basename(func.file_path)}:{func.line_number}")
        
        # Build parameter list with marshaling attributes
        param_lines = []
        for param in func.params:
            param_line = ""
            if param.marshaling_attr:
                if param.marshaling_attr == 'out':
                    param_line += f"out "
                else:
                    param_line += f"{param.marshaling_attr} "
            param_line += f"{param.csharp_type} {param.name}"
            param_lines.append(param_line)
            
        params_str = ", ".join(param_lines)
        
        # Generate DllImport attribute with EntryPoint
        lines.append(f'        [DllImport("Engine", EntryPoint = "{func.name}")]')
        
        # Generate function signature
        lines.append(f"        public static extern {func.return_type_csharp} {func.name}({params_str});")
        
        return lines


def main():
    parser = argparse.ArgumentParser(description="Generate C# bindings from C++ extern C functions")
    parser.add_argument("--source-dirs", nargs="+", 
                       default=["src/Engine"],
                       help="Directories to search for C++ source files")
    parser.add_argument("--output-dir", 
                       default="src/Game/Generated",
                       help="Output directory for generated C# files")
    parser.add_argument("--namespace",
                       default="EmEngine.Bindings",
                       help="C# namespace for generated code")
    parser.add_argument("--output-file",
                       default="EngineBindings.cs",
                       help="Output file name")
    
    args = parser.parse_args()
    
    # Get project root (assuming script is in resources/Scripts)
    script_dir = Path(__file__).parent
    project_root = script_dir.parent.parent
    
    # Convert relative paths to absolute
    source_dirs = [str(project_root / src_dir) for src_dir in args.source_dirs]
    output_dir = project_root / args.output_dir
    
    print("EmEngine Binding Generator")
    print(f"Scanning directories: {source_dirs}")
    print(f"Output: {output_dir / args.output_file}")
    print()
    
    # Parse C++ files
    parser = CppParser()
    print("Parsing C++ files...")
    functions = parser.parse_files(source_dirs)
    
    print(f"Found {len(functions)} extern C functions:")
    for func in sorted(functions, key=lambda f: f.name):
        print(f"  - {func.name} ({os.path.basename(func.file_path)}:{func.line_number})")
    print()
    
    # Generate C# code
    generator = CSharpGenerator(args.namespace)
    print("Generating C# bindings...")
    csharp_code = generator.generate(functions)
    
    # Ensure output directory exists
    output_dir.mkdir(parents=True, exist_ok=True)
    
    # Write output file
    output_file = output_dir / args.output_file
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(csharp_code)
        
    print(f"Generated {output_file}")
    print(f"Generated {len(functions)} function bindings")


if __name__ == "__main__":
    main()
