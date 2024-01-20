﻿using Astan;

namespace Sandbox
{
    public class Camera : Entity
    {
        public Entity OtherEntity;

        public float Speed;
        public float Timestep = 0.0f;

        void OnUpdate(float ts)
        {
            Timestep += ts;
            float speed = Speed;
            Vector3 velocity = Vector3.Zero;

            if (Input.IsKeyDown(KeyCode.Up))
                velocity.Y = 1f;
            else if (Input.IsKeyDown(KeyCode.Down))
                velocity.Y = -1f;

            if (Input.IsKeyDown(KeyCode.Left))
                velocity.X = -1f;
            else if (Input.IsKeyDown(KeyCode.Right))
                velocity.X = 1f;

            velocity *= speed;

            Vector3 translation = Translation;
            translation += velocity * ts;
            Translation = translation;

        }

    }
}
