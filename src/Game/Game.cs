using System;
using Input;
using FunctionsSetup;
using Entities;
using System.Runtime.InteropServices;
using System.Diagnostics;

public class Game
{
    public InputManager inputManager;
    public Player player;
    private static readonly Lazy<Game> lazy = new Lazy<Game>(() => new Game());
    public static Game Instance { get { return lazy.Value; } }

    private Stopwatch stopwatch;
    private float deltaTime;

    static void Main(string[] args)
    {
        Game.Instance.Initialize();
    }

    private Game()
    {
        inputManager = new InputManager(this);
        stopwatch = new Stopwatch();
    }

    public void Initialize()
    {
        if (NativeMethods.Initialize())
        {
            player = new Player();
            NativeMethods.LoadScene("resources/scene.yaml");
            NativeMethods.Start();
            stopwatch.Start(); // Start the stopwatch after initialization
        }
    }

    [UnmanagedCallersOnly(EntryPoint = "Game_Update")]
    public static void Update()
    {
        Game.Instance.CalculateDeltaTime();       // Calculate delta time for the frame
        NativeMethods.Update();                   // Update native methods (external code)
        Game.Instance.inputManager.Update();      // Update player input
        Game.Instance.player.Update(Game.Instance.deltaTime); // Update player with delta time
    }

    private void CalculateDeltaTime()
    {
        deltaTime = (float)stopwatch.Elapsed.TotalSeconds; // Convert elapsed time to seconds
        stopwatch.Restart();                               // Reset the stopwatch for the next frame
    }
}
