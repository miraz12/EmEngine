using System.Numerics;
using FunctionsSetup;
using System;

namespace Entities
{
    public class Player
    {
        public Vector3 position = new Vector3(0, 0, 0);
        public Vector3 forward = new Vector3(0, 0, 1);  // Default forward vector
        public Vector3 right = new Vector3(1, 0, 0);    // Default right vector
        private Vector3 velocity = new Vector3(0, 0, 0);
        private Vector3 forceDirection = new Vector3(0, 0, 0);  // Direction of force to apply

        private float maxForce = 50.0f;                 // Maximum force magnitude
        private float damping = 0.9f;                   // Damping factor for linear velocity
        private float maxSpeed = 10.0f;                 // Limit on speed
        private int entityId;

        public Player()
        {
            entityId = NativeMethods.CreateEntity("Player");
            NativeMethods.AddPositionComponent(entityId,
                                               new float[] { 0, 0, 0 },
                                               new float[] { 1, 1, 1 },
                                               new float[] { 0, 0, 0 });
            NativeMethods.AddGraphicsComponent(entityId, "gltf/Running/running.glb");
            NativeMethods.AddPhysicsComponent(entityId, 80.0f, 2);
        }

        public void Update(float dt)
        {
            // Calculate the force vector based on input direction
            Vector3 force = forceDirection * maxForce;

            // Update velocity based on force (acceleration = force * dt)
            velocity += force * dt;

            // Cap velocity to max speed
            if (velocity.Length() > maxSpeed)
            {
                velocity = Vector3.Normalize(velocity) * maxSpeed;
            }

            // Apply damping to simulate friction
            velocity *= damping;

            // Update Bullet with the calculated velocity
            NativeMethods.SetVelocity(entityId, velocity.X, velocity.Y, velocity.Z);

            // Reset force direction for the next frame (new input will set it again)
            forceDirection = Vector3.Zero;
        }

        // Method to set the force direction, called by InputManager
        public void SetForceDirection(Vector3 direction)
        {
            forceDirection = direction;
        }
    }
}
