using System;
using FunctionsSetup;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Numerics;

namespace Input
{
    enum KEY
    {
        Escape,
        W,
        A,
        S,
        D,
        F,
        O,
        Space,
        ArrowUp,
        ArrowDown,
        ArrowRight,
        ArrowLeft,
        Mouse1
    };


    public class InputManager
    {
        private Game _game;
        public InputManager(Game game)
        {
            _game = game;
        }
        private List<KEY> ConvertIntPtrToList(IntPtr vectorPointer, int count)
        {
            List<KEY> keys = new List<KEY>(count);
            for (int i = 0; i < count; i++)
            {
                IntPtr keyPointer = IntPtr.Add(vectorPointer, i * Marshal.SizeOf<KEY>());
                keys.Add(Marshal.PtrToStructure<KEY>(keyPointer));
            }
            return keys;
        }

        public void Update()
        {
            IntPtr vectorPointer;
            int count = NativeMethods.GetPressed(out vectorPointer);
            List<KEY> pressedKeys = ConvertIntPtrToList(vectorPointer, count);


            foreach (var key in pressedKeys)
            {
                switch (key)
                {
                    case KEY.W:
                        // _game.player.acceleration = Vector3.Add(_game.player.acceleration,
                        // _game.player.forward);
                        break;
                    case KEY.A:
                        // _game.player.acceleration = Vector3.Add(_game.player.acceleration,
                        //                                         _game.player.right);
                        break;
                    case KEY.S:
                        // _game.player.acceleration = Vector3.Add(_game.player.acceleration,
                        //                                         _game.player.forward);
                        break;
                    case KEY.D:
                        // _game.player.acceleration = Vector3.Add(_game.player.acceleration,
                        //                                         _game.player.right);
                        break;
                    case KEY.F:
                        break;
                    case KEY.O:
                        break;
                    case KEY.Space:
                        break;
                    case KEY.ArrowUp:
                        break;
                    case KEY.ArrowDown:
                        break;
                    case KEY.ArrowRight:
                        break;
                    case KEY.ArrowLeft:
                        break;
                    case KEY.Mouse1:
                        break;
                }
            }
            NativeMethods.ClearPressed();
        }
    }
}
