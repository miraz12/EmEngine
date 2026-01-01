using System;
using Input;
using EmEngine.Bindings;
using Entities;
using System.Runtime.InteropServices;
using System.Diagnostics;

public class Game
{
    public InputManager inputManager;
    public Player player;
    public FreeCameraController freeCamera;
    private static readonly Lazy<Game> lazy = new Lazy<Game>(() => new Game());
    public static Game Instance { get { return lazy.Value; } }

    private Stopwatch stopwatch;
    private float deltaTime;
    public bool useFreeCam = true; // Start with free camera for map inspection
    private bool lastPhysicsState = false;

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
        if (EngineApi.Initialize())
        {
            EngineApi.LoadMap("resources/map.txt", 2.0f);

            freeCamera = new FreeCameraController();
            player = new Player();

            EngineApi.SetMainCamera(freeCamera.GetEntityId());
            lastPhysicsState = false;

            EngineApi.Start();
            stopwatch.Start();
        }
    }

    [UnmanagedCallersOnly(EntryPoint = "Game_Update")]
    public static void Update()
    {
        Game.Instance.CalculateDeltaTime();
        EngineApi.Update();

        bool physicsEnabled = EngineApi.GetSimulatePhysics();
        Game.Instance.useFreeCam = !physicsEnabled;

        if (physicsEnabled != Game.Instance.lastPhysicsState)
        {
            if (physicsEnabled)
            {
                EngineApi.SetMainCamera(Game.Instance.player.GetEntityId());
            }
            else
            {
                EngineApi.SetMainCamera(Game.Instance.freeCamera.GetEntityId());
            }
            Game.Instance.lastPhysicsState = physicsEnabled;
        }

        Game.Instance.inputManager.Update();

        if (Game.Instance.useFreeCam)
        {
            Game.Instance.freeCamera.Update(Game.Instance.deltaTime);
        }
        else
        {
            Game.Instance.player.Update(Game.Instance.deltaTime);
        }
    }

    private void CalculateDeltaTime()
    {
        deltaTime = (float)stopwatch.Elapsed.TotalSeconds;
        stopwatch.Restart();
    }
}
