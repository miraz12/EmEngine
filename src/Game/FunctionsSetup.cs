using System;
using System.Runtime.InteropServices;
using Input;

namespace FunctionsSetup
{
    public class NativeMethods
    {
        public delegate void LoadSceneDelegate( string filename );
        public static LoadSceneDelegate LoadScene;

        public delegate int GetPressedDelegate( out IntPtr keys  );
        public static GetPressedDelegate GetPressed;

        [UnmanagedCallersOnly(EntryPoint = "InitializeFunctions")]
        public static void InitializeFunctions(IntPtr loadScenePtr, IntPtr getPressedPtr)
        {
            LoadScene = Marshal.GetDelegateForFunctionPointer<LoadSceneDelegate>(loadScenePtr);
            GetPressed = Marshal.GetDelegateForFunctionPointer<GetPressedDelegate>(getPressedPtr);
        }
    }
}
