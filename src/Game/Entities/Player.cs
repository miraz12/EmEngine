using System.Numerics;
using FunctionsSetup;
using System;

namespace Entities
{
    public class Player
    {
        public Vector3 position = new Vector3(0, 0, 0);
        public Vector3 forward = new Vector3(0, 0, 1);
        public Vector3 right = new Vector3(1, 0, 0);

        private Vector3 velocity = new Vector3(0, 0, 0);
        private Vector3 forceDirection = new Vector3(0, 0, 0);
        private float maxForce = 50.0f;
        private float damping = 0.9f;
        private float maxSpeed = 10.0f;
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
            NativeMethods.AddCameraComponent(entityId, true, new float[] { 0, 10.0F, 10.0F });
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

            // Reset force direction for the next frame
            forceDirection = Vector3.Zero;

        }

        public void SetForceDirection(Vector3 direction)
        {
            forceDirection = direction;
        }
    }
}
