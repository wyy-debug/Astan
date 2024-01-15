using System;
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
        void OnCreate()
        {
            Console.WriteLine($"Player.OnCreate - {ID}");
        }

        void OnUpdate(float ts)
        {

            float speed = 0.01f;
            Vector3 velocity = Vector3.Zero;

            if (Input.IsKeyDown(KeyCode.W))
                velocity.Y = 0.01f;
            else if (Input.IsKeyDown(KeyCode.S))
                velocity.Y = -0.01f;

            if (Input.IsKeyDown(KeyCode.A))
                velocity.X = -0.01f;
            else if (Input.IsKeyDown(KeyCode.D))
                velocity.X = 0.01f;

            velocity *= speed;


            Vector3 translation = Translation;
            translation += velocity * ts;
            Translation = translation;
            Console.WriteLine($"Player.OnUpdate:{translation}");

        }

    }
}
