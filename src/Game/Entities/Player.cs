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
        public Boolean jump = false;

        private Vector3 velocity = new Vector3(0, 0, 0);
        private Vector3 forceDirection = new Vector3(0, 0, 0);
        private float maxForce = 50.0f;
        private float damping = 0.9f;
        private float maxSpeed = 10.0f;
        private int entityId;

        // Jump-related variables
        private float jumpCooldown = 1.0f;      // Time required between jumps (in seconds)
        private float jumpTimer = 0f;           // Timer to track cooldown
        private bool canJump = true;            // Flag to control jump availability
        private float jumpForce = 10000f;         // Your existing jump force
        private bool wasOnGround = false;       // Track previous ground state

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
            bool isOnGround = NativeMethods.EntityOnGround(entityId);

            // Update jump timer
            if (!canJump)
            {
                jumpTimer += dt;
                if (jumpTimer >= jumpCooldown)
                {
                    jumpTimer = 0f;
                    canJump = true;
                }
            }

            // Reset canJump when landing
            if (isOnGround && !wasOnGround)
            {
                canJump = true;
                jumpTimer = 0f;
            }

            // Handle jumping
            if (jump && canJump && isOnGround)
            {
                NativeMethods.AddForce(entityId, 0, jumpForce, 0);
                canJump = false;
                jumpTimer = 0f;
            }
            jump = false;

            // Store ground state for next frame
            wasOnGround = isOnGround;

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

        // Optional: Methods to check jump status
        public bool CanJumpNow()
        {
            return canJump && NativeMethods.EntityOnGround(entityId);
        }

        public float GetJumpCooldownRemaining()
        {
            return canJump ? 0f : (jumpCooldown - jumpTimer);
        }
    }
}
