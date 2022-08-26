﻿using System;
using System.Runtime.InteropServices;

namespace Ouroboros
{
    public class GameObject : Object
    {
        private Scene m_Scene;
        private UInt64 m_InstanceID = 0;
        private Transform m_Transform = null;

        [DllImport("__Internal")] private static extern UInt64 CreateEntity(UInt32 sceneID);

        public GameObject()
        {
            m_Scene = SceneManager.GetActiveScene();
            m_InstanceID = CreateEntity(scene);
        }

        public GameObject(string name)
        {
            m_Scene = SceneManager.GetActiveScene();
            m_InstanceID = CreateEntity(scene);
            this.name = name;
        }

        public Scene scene
        {
            get { return m_Scene; }
        }

        public UInt64 GetInstanceID()
        {
            return m_InstanceID;
        }

        [DllImport("__Internal")] private static extern bool CheckEntityExists(UInt32 sceneID, UInt64 uuid);

        public static bool operator ==(GameObject lhs, GameObject rhs)
        {
            if (!ReferenceEquals(lhs, null) && ReferenceEquals(rhs, null)) // lhs is not null, but rhs is null
                return !CheckEntityExists(lhs.scene, lhs.GetInstanceID());
            if (ReferenceEquals(lhs, null) && !ReferenceEquals(rhs, null)) // lhs is null, but rhs is not null
                return !CheckEntityExists(rhs.scene, rhs.GetInstanceID());
            return ReferenceEquals(lhs, rhs);
        }

        public static bool operator !=(GameObject lhs, GameObject rhs)
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

        [DllImport("__Internal")] private static extern IntPtr GameObject_GetName(UInt32 sceneID, UInt64 uuid);
        [DllImport("__Internal")] private static extern void GameObject_SetName(UInt32 sceneID, UInt64 uuid, string newName);

        public string name
        {
            get
            {
                GCHandle stringPtr = GCHandle.FromIntPtr(GameObject_GetName(scene, GetInstanceID()));
                string name = (string)stringPtr.Target;
                stringPtr.Free();
                return name;
            }
            set { GameObject_SetName(scene, GetInstanceID(), value); }
        }

        //[DllImport("__Internal")] private static extern bool GameObject_GetActive(int id);
        //[DllImport("__Internal")] private static extern bool GameObject_GetActiveInHierarchy(int id);
        //[DllImport("__Internal")] private static extern void GameObject_SetActive(int id, bool value);

        //public bool activeInHierarchy
        //{
        //    get { return GameObject_GetActiveInHierarchy(m_InstanceID); }
        //}

        //public bool activeSelf
        //{
        //    get { return GameObject_GetActive(m_InstanceID); }
        //}

        //public void SetActive(bool value)
        //{
        //    GameObject_SetActive(m_InstanceID, value);
        //}
        //[DllImport("__Internal")] private static extern uint GameObject_GetLayer(int id);
        //[DllImport("__Internal")] private static extern void GameObject_SetLayer(int id, uint newLayer);

        //public uint layer
        //{
        //    get
        //    {
        //        return GameObject_GetLayer(m_InstanceID);
        //    }
        //    set { GameObject_SetLayer(m_InstanceID, value); }
        //}

        #region Script/Component

        public Transform transform
        {
            get { return m_Transform; }
        }

        //[DllImport("__Internal")] private static extern IntPtr AddScript(int id, string name_space, string name);
        //[DllImport("__Internal")] private static extern IntPtr AddComponentFromScript(int id, string name_space, string name);

        //public Component AddComponent(Type type)
        //{
        //    string name_space = "";
        //    if (type.Namespace != null)
        //        name_space = type.Namespace;

        //    IntPtr ptr = type.IsSubclassOf(typeof(MonoBehaviour))
        //                    ? AddScript(m_InstanceID, name_space, type.Name)
        //                    : AddComponentFromScript(m_InstanceID, name_space, type.Name);

        //    if (ptr == IntPtr.Zero)
        //        return null;
        //    return GCHandle.FromIntPtr(ptr).Target as Component;
        //}

        //public T AddComponent<T>() where T : Component
        //{
        //    Type type = typeof(T);
        //    string name_space = "";
        //    if (type.Namespace != null)
        //        name_space = type.Namespace;

        //    IntPtr ptr = type.IsSubclassOf(typeof(MonoBehaviour))
        //                    ? AddScript(m_InstanceID, name_space, type.Name)
        //                    : AddComponentFromScript(m_InstanceID, name_space, type.Name);

        //    if (ptr == IntPtr.Zero)
        //        return null;
        //    return GCHandle.FromIntPtr(ptr).Target as T;
        //}

        //[DllImport("__Internal")] private static extern IntPtr GetScript(int id, string name_space, string name);
        //[DllImport("__Internal")] private static extern IntPtr GetComponentFromScript(int id, string name_space, string name);

        //public Component GetComponent(Type type)
        //{
        //    string name_space = "";
        //    if (type.Namespace != null)
        //        name_space = type.Namespace;

        //    IntPtr ptr = type.IsSubclassOf(typeof(MonoBehaviour))
        //                    ? GetScript(m_InstanceID, name_space, type.Name)
        //                    : GetComponentFromScript(m_InstanceID, name_space, type.Name);

        //    if (ptr == IntPtr.Zero)
        //        return null;
        //    return GCHandle.FromIntPtr(ptr).Target as Component;
        //}

        //public T GetComponent<T>() where T : Component
        //{
        //    Type type = typeof(T);
        //    string name_space = "";
        //    if (type.Namespace != null)
        //        name_space = type.Namespace;

        //    IntPtr ptr = type.IsSubclassOf(typeof(MonoBehaviour))
        //                    ? GetScript(m_InstanceID, name_space, type.Name)
        //                    : GetComponentFromScript(m_InstanceID, name_space, type.Name);

        //    if (ptr == IntPtr.Zero)
        //        return null;
        //    return GCHandle.FromIntPtr(ptr).Target as T;
        //}

        //[DllImport("__Internal")] private static extern void RemoveScript(int id, string name_space, string name);
        //[DllImport("__Internal")] private static extern void RemoveComponentFromScript(int id, string name_space, string name);

        //public void RemoveComponent(Type type)
        //{
        //    if (type == typeof(Transform))
        //    {
        //        Debug.LogError("RemoveComponent failed: Cannot destroy transform!");
        //        return;
        //    }
        //    else if (type == typeof(Collider2D))
        //    {
        //        Debug.LogError("RemoveComponent failed: Cannot destroy Collider2D! Remove BoxCollider2D/CircleCollider2D instead");
        //        return;
        //    }

        //    string name_space = "";
        //    if (type.Namespace != null)
        //        name_space = type.Namespace;

        //    if (type.IsSubclassOf(typeof(MonoBehaviour)))
        //    {
        //        RemoveScript(m_InstanceID, name_space, type.Name);
        //    }
        //    else
        //    {
        //        RemoveComponentFromScript(m_InstanceID, name_space, type.Name);
        //    }
        //}

        //public void RemoveComponent<T>() where T : Component
        //{
        //    Type type = typeof(T);

        //    if (type == typeof(Transform))
        //    {
        //        Debug.LogError("RemoveComponent<T> failed: Cannot destroy transform!");
        //        return;
        //    }
        //    else if (type == typeof(Collider2D))
        //    {
        //        Debug.LogError("RemoveComponent failed: Cannot destroy Collider2D! Remove BoxCollider2D/CircleCollider2D instead");
        //        return;
        //    }

        //    string name_space = "";
        //    if (type.Namespace != null)
        //        name_space = type.Namespace;

        //    if (type.IsSubclassOf(typeof(MonoBehaviour)))
        //    {
        //        RemoveScript(m_InstanceID, name_space, type.Name);
        //    }
        //    else
        //    {
        //        RemoveComponentFromScript(m_InstanceID, name_space, type.Name);
        //    }
        //}

        #endregion Script/Component
    }
}