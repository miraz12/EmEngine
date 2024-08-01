using System;
using System.Runtime.InteropServices;
using Input;

namespace FunctionsSetup
{
    public class NativeMethods
    {
        [DllImport("Engine", EntryPoint = "LoadScene")]
        public static extern void LoadScene(string filename);
        [DllImport("Engine", EntryPoint = "Initialize")]
        public static extern void Initialize();
        [DllImport("Engine", EntryPoint = "Open")]
        public static extern bool Open();
        [DllImport("Engine", EntryPoint = "Update")]
        public static extern void Update();
    }
}
