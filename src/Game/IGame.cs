using System;
using System.Runtime.InteropServices;

public static class IGame {
    [UnmanagedCallersOnly(EntryPoint = "game_setup")]
    public static void game_setup()
    {
        Console.WriteLine("Game setup!");
    }

    [UnmanagedCallersOnly(EntryPoint = "game_initialize")]
    public static void game_initialize()
    {
        Game.Instance.Initialize();
    }

    [UnmanagedCallersOnly(EntryPoint = "game_update")]
    public static void game_update(float dt)
    {
        Game.Instance.Update(dt);
    }
}
