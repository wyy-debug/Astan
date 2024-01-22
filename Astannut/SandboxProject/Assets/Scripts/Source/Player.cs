using System;
using Astan;

namespace Sandbox
{
    public class Player : Entity
    {
        private TransformComponent m_Transform;
        private Rigidbody2DComponent m_RigidBody;

        public float Speed;
        public float Time = 0.0f;

        void OnCreate()
        {
            Console.WriteLine($"Player.OnCreate - {ID}");
            m_Transform = GetComponet<TransformComponent>();
            m_RigidBody = GetComponet<Rigidbody2DComponent>();
        }

        void OnUpdate(float ts)
        {

            Time += ts;

            float speed = Speed;
            Vector3 velocity = Vector3.Zero;

            if (Input.IsKeyDown(KeyCode.W))
                velocity.Y = 1f;
            else if (Input.IsKeyDown(KeyCode.S))
                velocity.Y = -1f;

            if (Input.IsKeyDown(KeyCode.A))
                velocity.X = -1f;
            else if (Input.IsKeyDown(KeyCode.D))
                velocity.X = 1f;

            Entity cameraEntity = FindEntityByName("Camera");
            if (cameraEntity != null)
            {
                Camera camera = cameraEntity.As<Camera>();
                
                if (Input.IsKeyDown(KeyCode.Q))
                    camera.DistanceFromPlayer += speed * ts;
                else if (Input.IsKeyDown(KeyCode.E))
                    camera.DistanceFromPlayer -= speed * ts;
            }

            // m_RigidBody.ApplyLinearImpulse(velocity.XY, Vector2.Zero, true);
            m_RigidBody.ApplyLinearImpulse(velocity.XY, true);
            velocity *= Speed;

            //Vector3 translation = m_Transform.Translation;
            //translation += velocity * ts;
            //m_Transform.Translation = translation;

        }

    }
}
