cmake_minimum_required(VERSION 3.16)
add_library(Game INTERFACE)

# Add custom target to generate C# bindings from C++ extern "C" functions
add_custom_target(
  GenerateBindings
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/Generated
  COMMAND python3 ${CMAKE_SOURCE_DIR}/resources/Scripts/generate_bindings.py
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Generating C# bindings from C++ extern C functions"
  VERBATIM
)

if(NOT EMSCRIPTEN)
  # Add custom target to publish the .NET project
  if(WIN32)
    add_custom_target(
      PublishDotNet ALL
      COMMAND dotnet publish Game.csproj -r win-x64 -c Release --packages
              .nuget/
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS Engine GenerateBindings
      COMMENT "Publishing .NET Native AOT project")
  endif(WIN32)
  if(LINUX)
    add_custom_target(
      PublishDotNet ALL
      COMMAND rm bin -r | dotnet publish Game.csproj -r linux-x64 -c Release
              --packages .nuget/
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS Engine GenerateBindings
      COMMENT "Publishing .NET Native AOT project")
  endif(LINUX)
else()
  add_custom_target(
    PublishDotNet ALL
    COMMAND dotnet publish Game_wasm.csproj -c Release -r browser-wasm
            --packages .nuget/
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS Engine GenerateBindings
    COMMENT "Publishing .NET Native AOT project WASM")
endif()
