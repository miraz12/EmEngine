using System;
using Input;
using FunctionsSetup;
using Entities;
using System.Runtime.InteropServices;
using System.Threading;

public class Game
{
    private InputManager inputManager;
    public Player player;

    static void Main(string[] args)
    {
        Game.Instance.Initialize();
    }

    private static readonly Lazy<Game> lazy =
        new Lazy<Game>(() => new Game());
    public static Game Instance { get { return lazy.Value; } }

    private Game()
    {
        inputManager = new InputManager(this);
        player = new Player();
    }

    public void Initialize()
    {
        NativeMethods.Initialize();
        NativeMethods.LoadScene("resources/scene.yaml");
        NativeMethods.Start();
    }

    [UnmanagedCallersOnly(EntryPoint = "Game_Update")]
    public static void Update()
    {
        NativeMethods.Update();
        // inputManager.Update(dt);
    }

}
