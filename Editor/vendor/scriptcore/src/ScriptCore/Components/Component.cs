using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Ouroboros
{
    public class Component : Object
    {
        private GameObject m_GameObject = null;
        private int m_ComponentID = -1;

        [DllImport("__Internal")] private static extern bool CheckComponentEnabled(UInt32 SceneID, UInt64 uuid, string name_space, string name);
        [DllImport("__Internal")] private static extern void SetComponentEnabled(UInt32 SceneID, UInt64 uuid, string name_space, string name, bool active);

        public bool enabled
        {
            get
            {
                Type type = GetType();
                string name = type.Name;
                string name_space = "";
                if (type.Namespace != null)
                    name_space = type.Namespace;
                return CheckComponentEnabled(gameObject.scene, gameObject.GetInstanceID(), name_space, name);
            }
            set
            {
                Type type = GetType();
                string name = type.Name;
                string name_space = "";
                if (type.Namespace != null)
                    name_space = type.Namespace;
                SetComponentEnabled(gameObject.scene, gameObject.GetInstanceID(), name_space, name, value);
            }
        }

        public GameObject gameObject
        {
            get { return m_GameObject; }
        }

        public Transform transform
        {
            get { return gameObject.transform; }
        }

        public string name
        {
            get { return gameObject.name; }
            set { gameObject.name = value; }
        }

        public int GetComponentID()
        {
            return m_ComponentID;
        }

        protected Component()
        {

        }

        [DllImport("__Internal")] private static extern bool CheckEntityExists(UInt32 sceneID, UInt64 uuid);

        public static bool operator ==(Component lhs, Component rhs)
        {
            if (ReferenceEquals(lhs, null) && ReferenceEquals(rhs, null)) // lhs is null, and rhs is null
                return true;
            if (!ReferenceEquals(lhs, null) && ReferenceEquals(rhs, null)) // lhs is not null, but rhs is null
                return !CheckEntityExists(lhs.gameObject.scene, lhs.gameObject.GetInstanceID());
            if (ReferenceEquals(lhs, null) && !ReferenceEquals(rhs, null)) // lhs is null, but rhs is not null
                return !CheckEntityExists(rhs.gameObject.scene, rhs.gameObject.GetInstanceID());
            return lhs.gameObject.GetInstanceID() == rhs.gameObject.GetInstanceID();
        }

        public static bool operator !=(Component lhs, Component rhs)
        {
            return !(lhs == rhs);
        }

        public override bool Equals(object obj)
        {
            return base.Equals(obj);
        }

        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        public override string ToString()
        {
            return (this == null) ? "null" : gameObject.name + " (" + GetType().FullName + ")";
        }

        #region Script/Component

        public Component AddComponent(Type type)
        {
            return gameObject.AddComponent(type);
        }

        public T AddComponent<T>() where T : Component
        {
            return gameObject.AddComponent<T>();
        }

        public Component GetComponent(Type type)
        {
            return gameObject.GetComponent(type);
        }

        public T GetComponent<T>() where T : Component
        {
            return gameObject.GetComponent<T>();
        }

        public Component[] GetComponentsInChildren(Type type, bool includeSelf = false)
        {
            return gameObject.GetComponentsInChildren(type, includeSelf);
        }

        public T[] GetComponentsInChildren<T>(bool includeSelf = false) where T : Component
        {
            return gameObject.GetComponentsInChildren<T>(includeSelf);
        }

        public void RemoveComponent(Type type)
        {
            gameObject.RemoveComponent(type);
        }

        public void RemoveComponent<T>() where T : Component
        {
            gameObject.RemoveComponent<T>();
        }

        #endregion Script/Component
    }
}
