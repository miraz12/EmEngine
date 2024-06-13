using System;
using System.Runtime.InteropServices;

public class NativeMethods
{
    public delegate void LoadSceneDelegate();
    public static LoadSceneDelegate LoadScene;
    [UnmanagedCallersOnly(EntryPoint = "InitializeFunctions")]
    public static void InitializeFunctions(IntPtr loadScenePtr)
    {
        LoadScene = Marshal.GetDelegateForFunctionPointer<LoadSceneDelegate>(loadScenePtr);
    }
}

public static class Game
{
    [UnmanagedCallersOnly(EntryPoint = "game_initialize")]
    public static void game_initialize()
    {
        NativeMethods.LoadScene();
    }

    [UnmanagedCallersOnly(EntryPoint = "game_update")]
    public static void game_update(float dt)
    {
    }
}
