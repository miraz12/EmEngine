using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Numerics;
using EmEngine.Bindings;

namespace Input
{
    enum KEY
    {
        Escape,
        W,
        A,
        S,
        D,
        Space,
        F,
        O,
        T,
        ArrowUp,
        ArrowDown,
        ArrowRight,
        ArrowLeft,
        Mouse1,
        Key1,
        Key2,
        Key3,
        Key4,
    };

    public class InputManager
    {
        private Game _game;
        private bool key1WasPressed = false;
        private bool key2WasPressed = false;
        private bool key3WasPressed = false;
        private bool key4WasPressed = false;

        public InputManager(Game game)
        {
            _game = game;
        }

        private List<int> ConvertIntPtrToList(IntPtr vectorPointer, int count)
        {
            List<int> keys = new List<int>(count);
            for (int i = 0; i < count; i++)
            {
                IntPtr keyPointer = IntPtr.Add(vectorPointer, i * sizeof(int));
                keys.Add(Marshal.PtrToStructure<int>(keyPointer));
            }
            return keys;
        }

        public void Update()
        {
            IntPtr vectorPointer;
            int count = EngineApi.GetPressed(out vectorPointer);
            List<int> pressedKeys = ConvertIntPtrToList(vectorPointer, count);

            // Default to no force in any direction
            Vector3 forceDirection = Vector3.Zero;
            
            // Track which number keys are currently pressed
            bool key1Pressed = false;
            bool key2Pressed = false;
            bool key3Pressed = false;
            bool key4Pressed = false;

            foreach (var key in pressedKeys)
            {
                switch ((KEY)key)
                {
                    case KEY.W:
                        forceDirection += _game.player.forward;
                        break;
                    case KEY.A:
                        forceDirection += _game.player.right;
                        break;
                    case KEY.S:
                        forceDirection -= _game.player.forward;
                        break;
                    case KEY.D:
                        forceDirection -= _game.player.right;
                        break;
                    case KEY.Space:
                        _game.player.RequestJump();
                        break;
                    case KEY.Key1:
                        key1Pressed = true;
                        break;
                    case KEY.Key2:
                        key2Pressed = true;
                        break;
                    case KEY.Key3:
                        key3Pressed = true;
                        break;
                    case KEY.Key4:
                        key4Pressed = true;
                        break;
                }
            }
            
            // Handle movement mode changes (only on key press, not while held)
            if (key1Pressed && !key1WasPressed)
            {
                _game.player.SetMovementMode(0); // Walk
            }
            if (key2Pressed && !key2WasPressed)
            {
                _game.player.SetMovementMode(1); // Jog
            }
            if (key3Pressed && !key3WasPressed)
            {
                _game.player.SetMovementMode(2); // Run
            }
            if (key4Pressed && !key4WasPressed)
            {
                _game.player.SetMovementMode(3); // Sprint
            }
            
            // Update previous key states
            key1WasPressed = key1Pressed;
            key2WasPressed = key2Pressed;
            key3WasPressed = key3Pressed;
            key4WasPressed = key4Pressed;

            // Normalize the direction if input is diagonal to maintain consistent force
            if (forceDirection.Length() > 0)
            {
                forceDirection = Vector3.Normalize(forceDirection);
            }

            // Set the direction of force to the player
            _game.player.SetForceDirection(forceDirection);
        }
    }
}
