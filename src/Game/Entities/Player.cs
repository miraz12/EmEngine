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
        private float rotationSpeed = 10.0f; // Speed at which the player rotates to face movement direction
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
            NativeMethods.AddCameraComponent(entityId, true, new float[] { 0, 10.0F, -10.0F });
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

            // Update facing direction only if we're moving
            if (velocity.Length() > 0.1f)
            {
                UpdateFacingDirection(dt);
                // NativeMethods.StartAnimation(entityId);
            } else {
                // NativeMethods.PauseAnimation(entityId);

            }

            // Get the current rotation as Euler angles
            float[] currentRotation = new float[3];
            // NativeMethods.GetRotation(entityId, currentRotation);

            // Calculate the desired yaw angle based on forward vector
            float targetYaw = (float)Math.Atan2(forward.X, forward.Z);

            // Update the rotation in the physics engine
            NativeMethods.SetRotation(entityId, targetYaw);

            // Update Bullet with the calculated velocity
            NativeMethods.SetVelocity(entityId, velocity.X, velocity.Y, velocity.Z);

            // Reset force direction for the next frame
            forceDirection = Vector3.Zero;
        }

        private void UpdateFacingDirection(float dt)
        {
            // Only update direction if we have movement
            if (velocity.Length() > 0.1f)
            {
                // Calculate the target forward direction from velocity
                Vector3 targetForward = Vector3.Normalize(new Vector3(velocity.X, 0, velocity.Z));

                // Smoothly interpolate current forward direction to target direction
                forward = Vector3.Normalize(Vector3.Lerp(forward, targetForward, rotationSpeed * dt));

                // Update right vector to be perpendicular to forward
                right = Vector3.Normalize(Vector3.Cross(Vector3.UnitY, forward));
            }
        }

        public void SetForceDirection(Vector3 direction)
        {
            forceDirection = direction;
        }

        public Vector3 GetFacingDirection()
        {
            return forward;
        }
    }
}
