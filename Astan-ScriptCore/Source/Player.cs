﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Astan;
using static System.Runtime.CompilerServices.RuntimeHelpers;

namespace Sandbox
{
    public class Player : Entity
    {
        private TransformComponent m_Transform;
        private Rigidbody2DComponent m_RigidBody;
        void OnCreate()
        {
            Console.WriteLine($"Player.OnCreate - {ID}");
            m_Transform = GetComponet<TransformComponent>();
            m_RigidBody = GetComponet<Rigidbody2DComponent>();
        }

        void OnUpdate(float ts)
        {

            float speed = 10f;
            Vector3 velocity = Vector3.Zero;

            if (Input.IsKeyDown(KeyCode.W))
                velocity.Y = 1f;
            else if (Input.IsKeyDown(KeyCode.S))
                velocity.Y = -1f;

            if (Input.IsKeyDown(KeyCode.A))
                velocity.X = -1f;
            else if (Input.IsKeyDown(KeyCode.D))
                velocity.X = 1f;

            // m_RigidBody.ApplyLinearImpulse(velocity.XY, Vector2.Zero, true);
            m_RigidBody.ApplyLinearImpulse(velocity.XY, true);
            velocity *= speed;

            //Vector3 translation = m_Transform.Translation;
            //translation += velocity * ts;
            //m_Transform.Translation = translation;

        }

    }
}
