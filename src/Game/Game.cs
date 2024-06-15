using System;
using Input;
using FunctionsSetup;
using Entities;

public class Game
{
    private InputManager inputManager;
    public Player player;

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
        NativeMethods.LoadScene("resources/scene.yaml");
    }

    public void Update(float dt)
    {
        inputManager.Update(dt);
    }

}
