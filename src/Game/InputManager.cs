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
    };

    public class InputManager
    {
        private Game _game;

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
                }
            }

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
