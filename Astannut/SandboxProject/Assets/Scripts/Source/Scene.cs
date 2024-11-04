using System;
using System.Collections.Generic;
using Astan;

namespace Sandbox
{
    public class Scene : Entity
    {
        private List<Entity> Point = new List<Entity>();
        private TcpClient tcpClient = new TcpClient();
        public float Time = 0.0f;
        public float step = 0.1f;


        void OnCreate()
        {
        }

        void OnUpdate(float ts)
        {
            tcpClient.Create();
            //tcpClient.Recive();
            /*Time += step;
            Entity entity = CreateEntity("Point");
            Vector3 color = new Vector3(1, 1, 1);
            Vector3 scale = new Vector3(0.1f, 0.1f, 0.0f); // only y  scale.y
            Vector3 translation = new Vector3(-5.0f, 0.5f, 0.0f);  // only X translation.y = scale.y / 2
            translation.X = translation.X + Time;
            translation.Y = Time;
            entity.AddSpriteRenderer(ref color);
            TransformComponent transform = entity.GetComponet<TransformComponent>();
            transform.Scale = scale;
            transform.Translation = translation;*/

        }

    }
}
