using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Astan;

namespace Sandbox
{
    public class Scene : Entity
    {
        private List<Entity> Point = new List<Entity>();
        private TcpClient tcpClient = new TcpClient();
        public float Time = 0.0f;
        public float step = 0.2f;


        void OnCreate()
        {
            tcpClient.Create();
        }

        void OnUpdate(float ts)
        {
            tcpClient.Recive();

            if (tcpClient.damge != null)
            {
                if (tcpClient.damge.Value > 0)
                {
                    Console.WriteLine("damge create");
                    Time += step;
                    Entity entity = CreateEntity(tcpClient.damge.WeaponName);
                    Vector3 color = new Vector3(0, 0, 1);
                    Vector3 scale = new Vector3(0.1f, 0.1f, 0.0f); // only y  scale.y
                    Vector3 translation = new Vector3(-5.0f, 0.5f, 0.0f);  // only X translation.y = scale.y / 2
                    translation.X = translation.X + Time;
                    float x = tcpClient.damge.Value / 20.0f;
                    translation.Y = x;
                    entity.AddSpriteRenderer(ref color);
                    TransformComponent transform = entity.GetComponet<TransformComponent>();
                    transform.Scale = scale;
                    transform.Translation = translation;
                    tcpClient.damge = null;
                }
                else
                {
                    Console.WriteLine("damge create");
                    Time += step;
                    Entity entity = CreateEntity(tcpClient.damge.WeaponName);
                    Vector3 color = new Vector3(1, 0, 0);
                    Vector3 scale = new Vector3(0.1f, 0.1f, 0.0f); // only y  scale.y
                    Vector3 translation = new Vector3(-5.0f, 0.5f, 0.0f);  // only X translation.y = scale.y / 2
                    translation.X = translation.X + Time;
                    float x = tcpClient.damge.Value / 20.0f;
                    translation.Y = x;
                    entity.AddSpriteRenderer(ref color);
                    TransformComponent transform = entity.GetComponet<TransformComponent>();
                    transform.Scale = scale;
                    transform.Translation = translation;
                    tcpClient.damge = null;
                }
            }

        }

    }
}
