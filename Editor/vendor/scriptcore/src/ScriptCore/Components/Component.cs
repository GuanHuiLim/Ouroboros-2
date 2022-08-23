using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Ouroboros
{
    public class Component : Object
    {
        private GameObject m_GameObject = null;
        private int m_ComponentID = -1;

        //[DllImport("__Internal")] private static extern bool CheckComponentEnabled(int instanceID, string name_space, string name);
        //[DllImport("__Internal")] private static extern void SetComponentEnabled(int instanceID, string name_space, string name, bool active);

        //public bool enabled
        //{
        //    get
        //    {
        //        Type type = GetType();
        //        string name = type.Name;
        //        string name_space = "";
        //        if (type.Namespace != null)
        //            name_space = type.Namespace;
        //        return CheckComponentEnabled(gameObject.GetInstanceID(), name_space, name);
        //    }
        //    set
        //    {
        //        Type type = GetType();
        //        string name = type.Name;
        //        string name_space = "";
        //        if (type.Namespace != null)
        //            name_space = type.Namespace;
        //        SetComponentEnabled(gameObject.GetInstanceID(), name_space, name, value);
        //    }
        //}

        //public GameObject gameObject
        //{
        //    get { return m_GameObject; }
        //}

        //public Transform transform
        //{
        //    get { return gameObject.transform; }
        //}

        //public string name
        //{
        //    get { return gameObject.name; }
        //    set { gameObject.name = value; }
        //}

        public int GetComponentID()
        {
            return m_ComponentID;
        }

        protected Component()
        {

        }

        //[DllImport("__Internal")] private static extern bool CheckEntityExists(int id);

        //public static bool operator ==(Component lhs, Component rhs)
        //{
        //    if (ReferenceEquals(lhs, null) && ReferenceEquals(rhs, null)) // both lhs and rhs is null
        //    {
        //        return true;
        //    }
        //    if (!ReferenceEquals(lhs, null) && ReferenceEquals(rhs, null)) // lhs is not null, but rhs is null
        //    {
        //        return !CheckEntityExists(lhs.gameObject.GetInstanceID()) || !ReferenceEquals(lhs.GetComponent(lhs.GetType()), lhs);//lhs.GetComponent(lhs.GetType()) != lhs;
        //    }
        //    if (ReferenceEquals(lhs, null) && !ReferenceEquals(rhs, null)) // lhs is null, but rhs is not null
        //    {
        //        return !CheckEntityExists(rhs.gameObject.GetInstanceID()) || !ReferenceEquals(rhs.GetComponent(rhs.GetType()), rhs);//rhs.GetComponent(rhs.GetType()) != rhs;
        //    }
        //    return ReferenceEquals(lhs, rhs);
        //}

        //public static bool operator !=(Component lhs, Component rhs)
        //{
        //    return !(lhs == rhs);
        //}

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
            return (this == null) ? "null" : base.ToString();
        }

        //#region Script/Component

        //public Component AddComponent(Type type)
        //{
        //    return gameObject.AddComponent(type);
        //}

        //public T AddComponent<T>() where T : Component
        //{
        //    return gameObject.AddComponent<T>();
        //}

        //public Component GetComponent(Type type)
        //{
        //    return gameObject.GetComponent(type);
        //}

        //public T GetComponent<T>() where T : Component
        //{
        //    return gameObject.GetComponent<T>();
        //}

        //public void RemoveComponent(Type type)
        //{
        //    gameObject.RemoveComponent(type);
        //}

        //public void RemoveComponent<T>() where T : Component
        //{
        //    gameObject.RemoveComponent<T>();
        //}

        //#endregion Script/Component
    }
}
