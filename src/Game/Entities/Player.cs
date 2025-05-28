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
        
        // Force applied for movement (Newtons) - average human can produce ~400-600N horizontally
        private float maxForce = 500.0f;
        
        // Movement speeds in m/s based on real human locomotion data:
        // Walking: 1.2-1.4 m/s, Jogging: 2.5-3.5 m/s, Running: 4.5-6.0 m/s, Sprint: 8-12 m/s
        private float walkSpeed = 1.4f;      // Comfortable walking pace
        private float jogSpeed = 3.0f;       // Light jogging
        private float runSpeed = 5.5f;       // Running pace
        private float sprintSpeed = 9.0f;    // Sprint speed (Olympic average ~10.5 m/s)
        
        // Current movement mode
        private enum MovementMode { Walk, Jog, Run, Sprint }
        private MovementMode currentMode = MovementMode.Run;
        
        // Acceleration and deceleration (m/s²) - realistic human values
        private float acceleration = 8.0f;   // Time to reach max speed ~0.7 seconds
        private float deceleration = 12.0f;  // Stopping is faster than starting
        private float airControl = 0.3f;     // Reduced control while airborne
        
        // Friction coefficients
        private float groundFriction = 0.85f;  // Ground friction (concrete/asphalt)
        private float airResistance = 0.02f;   // Air resistance at low speeds
        
        // Rotation
        private float rotationSpeed = 8.0f;
        
        // === JUMP PHYSICS ===
        // Jump height: 0.4m (average human vertical jump) requires ~2.8 m/s initial velocity
        // Formula: v = sqrt(2 * g * h) where g = 9.81 m/s², h = 0.4m
        private float jumpVelocity = 2.8f;    // Realistic vertical jump velocity
        private bool jumpRequested = false;
        private bool canJump = true;
        private float jumpCooldown = 0.15f;   // Minimum time between jumps
        private float jumpTimer = 0.0f;
        private int entityId;
        
        // Debug tracking
        private bool wasMoving = false;

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
            if (!canJump)
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
            if (jumpRequested && onGround && canJump)
            {
                velocity.Y = jumpVelocity;
                canJump = false;
                jumpTimer = 0.0f;
            }
            
            // === PHYSICS-BASED MOVEMENT ===
            // Get current target speed based on movement mode
            float targetSpeed = GetTargetSpeed();
            
            // Debug: Print current movement info when moving
            bool isMoving = forceDirection.Length() > 0.1f;
            if (isMoving && !wasMoving)
            {
                System.Console.WriteLine($"Movement Mode: {currentMode} | Target Speed: {targetSpeed:F1} m/s");
            }
            wasMoving = isMoving;
            
            // Calculate horizontal velocity components
            Vector3 horizontalVelocity = new Vector3(velocity.X, 0, velocity.Z);
            float currentSpeed = horizontalVelocity.Length();
            
            if (forceDirection.Length() > 0.1f)
            {
                // Normalize input direction
                Vector3 inputDirection = Vector3.Normalize(forceDirection);
                
                // Calculate target velocity
                Vector3 targetVelocity = inputDirection * targetSpeed;
                
                // Use appropriate acceleration based on ground contact
                float currentAcceleration = onGround ? acceleration : acceleration * airControl;
                
                // Apply acceleration toward target velocity
                Vector3 velocityDifference = targetVelocity - horizontalVelocity;
                Vector3 accelerationVector = velocityDifference * currentAcceleration * dt;
                
                // Clamp acceleration to prevent overshooting
                if (accelerationVector.Length() > velocityDifference.Length())
                {
                    accelerationVector = velocityDifference;
                }
                
                // Apply acceleration
                velocity.X += accelerationVector.X;
                velocity.Z += accelerationVector.Z;
            }
            else
            {
                // No input - apply deceleration/friction
                if (onGround)
                {
                    // Ground friction
                    float frictionForce = deceleration * groundFriction * dt;
                    if (currentSpeed > 0)
                    {
                        Vector3 frictionDirection = -Vector3.Normalize(horizontalVelocity);
                        Vector3 frictionVector = frictionDirection * Math.Min(frictionForce, currentSpeed);
                        
                        velocity.X += frictionVector.X;
                        velocity.Z += frictionVector.Z;
                    }
                }
                else
                {
                    // Air resistance (much lighter)
                    float airResistanceForce = currentSpeed * currentSpeed * airResistance * dt;
                    if (currentSpeed > 0)
                    {
                        Vector3 resistanceDirection = -Vector3.Normalize(horizontalVelocity);
                        Vector3 resistanceVector = resistanceDirection * Math.Min(airResistanceForce, currentSpeed);
                        
                        velocity.X += resistanceVector.X;
                        velocity.Z += resistanceVector.Z;
                    }
                }
            }
            
            // Calculate final horizontal speed for animation
            float finalHorizontalSpeed = (float)Math.Sqrt(velocity.X * velocity.X + velocity.Z * velocity.Z);

            // Update facing direction and animations
            if (finalHorizontalSpeed > 0.1f)
            {
                UpdateFacingDirection(dt);
                
                // Animation based on speed thresholds
                if (finalHorizontalSpeed > walkSpeed * 0.5f && onGround)
                {
                    EngineApi.StartAnimation(entityId);
                }
                else
                {
                    EngineApi.PauseAnimation(entityId);
                }
            }
            else
            {
                EngineApi.PauseAnimation(entityId);
            }

            // Update rotation based on movement direction
            float targetYaw = (float)Math.Atan2(forward.X, forward.Z);
            EngineApi.SetRotation(entityId, targetYaw);

            // Apply velocity to physics engine
            EngineApi.SetVelocity(entityId, velocity.X, velocity.Y, velocity.Z);
            
            // Apply additional force for more responsive movement
            if (forceDirection.Length() > 0.1f && onGround)
            {
                Vector3 forceVector = Vector3.Normalize(forceDirection) * maxForce;
                EngineApi.AddForce(entityId, forceVector.X, 0, forceVector.Z);
            }

            // Reset inputs for next frame
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
        
        private float GetTargetSpeed()
        {
            return currentMode switch
            {
                MovementMode.Walk => walkSpeed,
                MovementMode.Jog => jogSpeed,
                MovementMode.Run => runSpeed,
                MovementMode.Sprint => sprintSpeed,
                _ => runSpeed
            };
        }
        
        public void SetMovementMode(int mode)
        {
            currentMode = mode switch
            {
                0 => MovementMode.Walk,
                1 => MovementMode.Jog,
                2 => MovementMode.Run,
                3 => MovementMode.Sprint,
                _ => MovementMode.Run
            };
        }
        
        public float GetCurrentSpeed()
        {
            return (float)Math.Sqrt(velocity.X * velocity.X + velocity.Z * velocity.Z);
        }
        
        public string GetMovementModeString()
        {
            return currentMode.ToString();
        }
    }
}
