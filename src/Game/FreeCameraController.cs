using System;
using System.Numerics;
using EmEngine.Bindings;

namespace Entities
{
    public class FreeCameraController
    {
        private int cameraEntity;
        private Vector3 position = new Vector3(0, 10, -20);
        private Vector3 front = new Vector3(0, 0, 1);
        private Vector3 up = new Vector3(0, 1, 0);
        private Vector3 right = new Vector3(1, 0, 0);

        private float yaw = 90.0f;
        private float pitch = -20.0f;

        private float moveSpeed = 10.0f;
        private float fastMoveSpeed = 30.0f;
        private float mouseSensitivity = 0.1f;

        private Vector3 moveDirection = Vector3.Zero;
        private bool fastMode = false;

        public FreeCameraController()
        {
            cameraEntity = EngineApi.CreateEntity("FreeCamera");

            EngineApi.AddPositionComponent(cameraEntity,
                new float[] { position.X, position.Y, position.Z },
                new float[] { 1, 1, 1 },
                new float[] { 0, 0, 0 });

            EngineApi.AddCameraComponent(cameraEntity, true, new float[] { 0, 0, 0 });

            UpdateCameraVectors();
        }

        public void Update(float dt)
        {
            float currentSpeed = fastMode ? fastMoveSpeed : moveSpeed;

            if (moveDirection.Length() > 0.1f)
            {
                Vector3 normalizedMove = Vector3.Normalize(moveDirection);
                position += normalizedMove * currentSpeed * dt;
            }

            EngineApi.AddPositionComponent(cameraEntity,
                new float[] { position.X, position.Y, position.Z },
                new float[] { 1, 1, 1 },
                new float[] { 0, 0, 0 });

            EngineApi.SetCameraOrientation(cameraEntity,
                front.X, front.Y, front.Z,
                up.X, up.Y, up.Z);

            // Reset movement for next frame
            moveDirection = Vector3.Zero;
            fastMode = false;
        }

        public void HandleMouseMovement(float deltaX, float deltaY)
        {
            deltaX *= mouseSensitivity;
            deltaY *= mouseSensitivity;

            yaw += deltaX;
            pitch += deltaY;

            // Constrain pitch to prevent screen flip
            if (pitch > 89.0f)
                pitch = 89.0f;
            if (pitch < -89.0f)
                pitch = -89.0f;

            UpdateCameraVectors();
        }

        private void UpdateCameraVectors()
        {
            Vector3 newFront;
            newFront.X = (float)(Math.Cos(DegreesToRadians(yaw)) * Math.Cos(DegreesToRadians(pitch)));
            newFront.Y = (float)Math.Sin(DegreesToRadians(pitch));
            newFront.Z = (float)(Math.Sin(DegreesToRadians(yaw)) * Math.Cos(DegreesToRadians(pitch)));

            front = Vector3.Normalize(newFront);
            right = Vector3.Normalize(Vector3.Cross(front, Vector3.UnitY));
            up = Vector3.Normalize(Vector3.Cross(right, front));
        }

        public void MoveForward()
        {
            moveDirection += front;
        }

        public void MoveBackward()
        {
            moveDirection -= front;
        }

        public void MoveLeft()
        {
            moveDirection -= right;
        }

        public void MoveRight()
        {
            moveDirection += right;
        }

        public void MoveUp()
        {
            moveDirection += Vector3.UnitY;
        }

        public void MoveDown()
        {
            moveDirection -= Vector3.UnitY;
        }

        public void SetFastMode(bool fast)
        {
            fastMode = fast;
        }

        private float DegreesToRadians(float degrees)
        {
            return degrees * (float)Math.PI / 180.0f;
        }

        public Vector3 GetPosition()
        {
            return position;
        }

        public Vector3 GetFront()
        {
            return front;
        }

        public int GetEntityId()
        {
            return cameraEntity;
        }
    }
}
