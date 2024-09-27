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
        unsafe public static extern bool Initialize();
        [DllImport("Engine", EntryPoint = "Open")]
        public static extern bool Open();
        [DllImport("Engine", EntryPoint = "Update")]
        public static extern void Update();
        [DllImport("Engine", EntryPoint = "Start")]
        public static extern void Start();
        [DllImport("Engine", EntryPoint = "GetPressed")]
        public static extern int GetPressed(out IntPtr vec);
        [DllImport("Engine", EntryPoint = "ClearPressed")]
        public static extern void ClearPressed();
    }
}
