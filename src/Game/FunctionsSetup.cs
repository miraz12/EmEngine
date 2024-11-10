using System;
using System.Runtime.InteropServices;
using Input;

namespace FunctionsSetup
{
    public class NativeMethods
    {

        [DllImport("Engine", EntryPoint = "LoadScene")]
        public static extern void LoadScene(string filename);
        [DllImport("Engine", EntryPoint = "Initialize")]
        unsafe public static extern bool Initialize();
        [DllImport("Engine", EntryPoint = "Open")]
        public static extern bool Open();
        [DllImport("Engine", EntryPoint = "Update")]
        public static extern void Update();
        [DllImport("Engine", EntryPoint = "Start")]
        public static extern void Start();
        [DllImport("Engine", EntryPoint = "SetVelocity")]
        public static extern void SetVelocity(int entity, float x, float y, float z);
        [DllImport("Engine", EntryPoint = "AddImpulse")]
        public static extern void AddImpulse(int entity, float x, float y, float z);
        [DllImport("Engine", EntryPoint = "AddForce")]
        public static extern void AddForce(int entity, float x, float y, float z);
        [DllImport("Engine", EntryPoint = "SetAcceleration")]
        public static extern void SetAcceleration(int entity, float x, float y, float z);
        [DllImport("Engine", EntryPoint = "GetPressed")]
        public static extern int GetPressed(out IntPtr vec);
        [DllImport("Engine", EntryPoint = "EntityOnGround")]
        public static extern Boolean EntityOnGround(int entity);

        [DllImport("Engine", EntryPoint = "AddPositionComponent")]
        public static extern void AddPositionComponent(int entity,
                                                       [In] float[] position,
                                                       [In] float[] scale,
                                                       [In] float[] rotation);
        [DllImport("Engine", EntryPoint = "AddPhysicsComponent")]
        public static extern void AddPhysicsComponent(int entity, float mass, int type);
        [DllImport("Engine", EntryPoint = "AddGraphicsComponent")]
        public static extern void AddGraphicsComponent(int entity, string model);
        [DllImport("Engine", EntryPoint = "CreateEntity")]
        public static extern int CreateEntity(string name);
    }
}
