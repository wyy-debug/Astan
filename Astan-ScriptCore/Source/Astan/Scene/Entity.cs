using System;
using System.Runtime.CompilerServices;

namespace Astan {
    
    public class Entity
    {
        protected Entity() { ID = 0; }

        internal Entity(ulong id)
        {
            ID = id;
        }

        public readonly ulong ID;


        public Vector3 Translation
        { 
            get 
            {
                InternalCalls.TransformComponent_GetTranslation(ID, out Vector3 result);
                return result;
            }
            set 
            {
                InternalCalls.TransformComponent_SetTranslation(ID, ref value);
            }
        }

        public bool HasComponent<T>() where T : Component, new() 
        {
            Type componentType = typeof(T);
            return InternalCalls.Entity_HasComponent(ID, componentType);
        }

        public T GetComponet<T>() where T: Component, new()
        {
            if(!HasComponent<T>())
                return null;

            T component =  new T() { Entity = this };
            return component;
        }

        public Entity FindEntityByName(string name)
        {
            ulong entity = InternalCalls.Entity_FindEntityByName(name);
            if (entity == 0)
                return null;
            return new Entity(entity);
        }
        
        public T As<T>() where T : Entity, new() 
        {
            object instance =  InternalCalls.GetScriptInstance(ID);
            return instance as T;
            
        }
    }
}
 