using System.Numerics;
using EmEngine.Bindings;
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
        // Force applied for movement - only used for minor adjustments now
        private float maxForce = 100.0f;
        // Damping factor (0-1) - simulates friction/air resistance
        private float damping = 0.1f;
        // Maximum movement speed in meters per second (sprint speed)
        // Walking: ~1.4 m/s, Jogging: ~3.0 m/s, Running: ~5-8 m/s, Sprint: ~9-11 m/s
        private float maxSpeed = 7.0f;
        // Speed at which the player rotates to face movement direction
        private float rotationSpeed = 8.0f;
        // Jump parameters
        private float jumpVelocity = 7.0f;
        private bool jumpRequested = false;
        private bool canJump = true; // Should start as true
        private float jumpCooldown = 0.2f;
        private float jumpTimer = 0.0f;
        private int entityId;

        public Player()
        {
            entityId = EngineApi.CreateEntity("Player");
            
            // Position the player slightly above ground to avoid initial collisions
            EngineApi.AddPositionComponent(entityId,
                                              new float[] { 0, 1.0f, 0 },
                                              new float[] { 1, 1, 1 },
                                              new float[] { 0, 0, 0 });
                                              
            // Add the character model
            EngineApi.AddGraphicsComponent(entityId, "gltf/Running/running.glb");
            
            // Add physics with a CAPSULE shape (type=2)
            // Set mass to 70kg (average human mass)
            EngineApi.AddPhysicsComponent(entityId, 70.0f, 2);
            
            // Position the camera to follow from behind and slightly above
            EngineApi.AddCameraComponent(entityId, true, new float[] { 0, 8.0F, -10.0F });
            
            // Start with character at rest
            EngineApi.PauseAnimation(entityId);
            
            // Ensure canJump is true after initialization
            canJump = true;
        }

        public void Update(float dt)
        {
            // Check if player is on ground for better movement control
            bool onGround = EngineApi.EntityOnGround(entityId);
            
            // Handle jump cooldown timer
            if (!canJump && !onGround)
            {
                jumpTimer += dt;
                if (jumpTimer >= jumpCooldown)
                {
                    canJump = true;
                    jumpTimer = 0.0f;
                }
            }
            
            // Reset jump ability when landing
            if (onGround && !canJump && velocity.Y <= 0)
            {
                canJump = true;
                jumpTimer = 0.0f;
            }
            
            // Apply jump if requested and player is on ground
            if (jumpRequested)
            {

                if (onGround && canJump)
                {
                    velocity.Y = jumpVelocity;
                    canJump = false;  // Prevent repeated jumps until landing or cooldown
                    jumpTimer = 0.0f;
                }
            }
            
            // Calculate the force vector based on input direction
            // Use direct velocity control approach for more consistent speed
            
            // If we have input, set the velocity directly to max speed in that direction
            if (forceDirection.Length() > 0.1f)
            {
                // Normalize the direction vector
                Vector3 normalizedDirection = Vector3.Normalize(forceDirection);
                
                // Get current Y velocity before overwriting
                float currentYVelocity = velocity.Y;
                
                // Set velocity directly to max speed in the input direction
                velocity = normalizedDirection * maxSpeed;
                
                // Keep the original Y velocity component (for jumps/falls)
                velocity.Y = currentYVelocity;
            }
            else
            {
                // If no input, apply damping to slow down
                float currentDamping = onGround ? damping : 0.98f;
                
                // Only apply horizontal damping in air
                if (onGround)
                {
                    velocity.X *= currentDamping;
                    velocity.Z *= currentDamping;
                }
                else
                {
                    velocity *= currentDamping;
                }
            }
            
            // Calculate horizontal speed for other checks
            float horizontalSpeed = (float)Math.Sqrt(velocity.X * velocity.X + velocity.Z * velocity.Z);

            // Only animate and update facing direction if the player is moving on ground
            if (horizontalSpeed > 0.1f)
            {
                UpdateFacingDirection(dt);
                
                // Animation thresholds for running
                // Always play animation when moving - this is a running character
                if (horizontalSpeed > 1.0f && onGround)
                {
                    EngineApi.StartAnimation(entityId);
                }
                else if (horizontalSpeed < 0.5f)
                {
                    EngineApi.PauseAnimation(entityId);
                }
            }
            else
            {
                EngineApi.PauseAnimation(entityId);
            }

            // Calculate the desired yaw angle based on forward vector
            float targetYaw = (float)Math.Atan2(forward.X, forward.Z);

            // Update the rotation in the physics engine
            EngineApi.SetRotation(entityId, targetYaw);

            // Set the velocity directly in bullet physics
            EngineApi.SetVelocity(entityId, velocity.X, velocity.Y, velocity.Z);
            
            // Also apply forces to help overcome any other physics constraints
            // This provides an additional push in the movement direction
            if (forceDirection.Length() > 0.1f)
            {
                EngineApi.AddForce(entityId, 
                                      forceDirection.X * maxForce * 2.0f,
                                      0, 
                                      forceDirection.Z * maxForce * 2.0f);
            }

            // Reset jump request and force direction for the next frame
            jumpRequested = false;
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

        public void RequestJump()
        {
            jumpRequested = true;
        }

        public Vector3 GetFacingDirection()
        {
            return forward;
        }
    }
}
