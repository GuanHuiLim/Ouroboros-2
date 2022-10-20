using System;
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
            m_Transform = GetComponent<Transform>();
        }

        public GameObject(string name)
        {
            m_Scene = SceneManager.GetActiveScene();
            m_InstanceID = CreateEntity(scene);
            m_Transform = GetComponent<Transform>();
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
            if (ReferenceEquals(lhs, null) && ReferenceEquals(rhs, null)) // lhs is null, and rhs is null
                return true;
            if (!ReferenceEquals(lhs, null) && ReferenceEquals(rhs, null)) // lhs is not null, but rhs is null
                return !CheckEntityExists(lhs.scene, lhs.GetInstanceID());
            if (ReferenceEquals(lhs, null) && !ReferenceEquals(rhs, null)) // lhs is null, but rhs is not null
                return !CheckEntityExists(rhs.scene, rhs.GetInstanceID());
            return lhs.m_InstanceID == rhs.m_InstanceID;
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

        public override string ToString()
        {
            return (this == null) ? "null" : name + " (GameObject)";
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

        [DllImport("__Internal")] private static extern bool GameObject_GetActive(UInt32 sceneID, UInt64 uuid);
        [DllImport("__Internal")] private static extern bool GameObject_GetActiveInHierarchy(UInt32 sceneID, UInt64 uuid);
        [DllImport("__Internal")] private static extern void GameObject_SetActive(UInt32 sceneID, UInt64 uuid, bool value);

        public bool activeInHierarchy
        {
            get { return GameObject_GetActiveInHierarchy(scene, GetInstanceID()); }
        }

        public bool activeSelf
        {
            get { return GameObject_GetActive(scene, GetInstanceID()); }
        }

        public void SetActive(bool value)
        {
            GameObject_SetActive(scene, GetInstanceID(), value);
        }

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
            get 
            {
                if (this == null)
                    throw new NullReferenceException();
                return m_Transform; 
            }
        }

        [DllImport("__Internal")] private static extern IntPtr AddScript(UInt32 SceneID, UInt64 uuid, string name_space, string name);
        [DllImport("__Internal")] private static extern IntPtr AddComponentFromScript(UInt32 SceneID, UInt64 uuid, string name_space, string name);

        public Component AddComponent(Type type)
        {
            string name_space = "";
            if (type.Namespace != null)
                name_space = type.Namespace;

            IntPtr ptr = IntPtr.Zero;
            if (type.IsSubclassOf(typeof(MonoBehaviour)))
            {
                ptr = AddScript(scene, GetInstanceID(), name_space, type.Name);
            }
            else
            {
                ptr = AddComponentFromScript(scene, GetInstanceID(), name_space, type.Name);
            }

            if (ptr == IntPtr.Zero)
                return null;
            return GCHandle.FromIntPtr(ptr).Target as Component;
        }

        public T AddComponent<T>() where T : Component
        {
            Type type = typeof(T);
            string name_space = "";
            if (type.Namespace != null)
                name_space = type.Namespace;

            IntPtr ptr = IntPtr.Zero;
            if (type.IsSubclassOf(typeof(MonoBehaviour)))
            {
                ptr = AddScript(scene, GetInstanceID(), name_space, type.Name);
            }
            else
            {
                ptr = AddComponentFromScript(scene, GetInstanceID(), name_space, type.Name);
            }

            if (ptr == IntPtr.Zero)
                return null;
            return GCHandle.FromIntPtr(ptr).Target as T;
        }

        [DllImport("__Internal")] private static extern IntPtr GetScript(UInt32 SceneID, UInt64 uuid, string name_space, string name);
        [DllImport("__Internal")] private static extern IntPtr GetComponentFromScript(UInt32 SceneID, UInt64 uuid, string name_space, string name);

        public Component GetComponent(Type type)
        {
            string name_space = "";
            if (type.Namespace != null)
                name_space = type.Namespace;

            IntPtr ptr = IntPtr.Zero;
            if (type.IsSubclassOf(typeof(MonoBehaviour)))
            {
                ptr = GetScript(scene, GetInstanceID(), name_space, type.Name);
            }
            else
            {
                ptr = GetComponentFromScript(scene, GetInstanceID(), name_space, type.Name);
            }

            if (ptr == IntPtr.Zero)
                return null;
            return GCHandle.FromIntPtr(ptr).Target as Component;
        }

        public T GetComponent<T>() where T : Component
        {
            Type type = typeof(T);
            string name_space = "";
            if (type.Namespace != null)
                name_space = type.Namespace;

            IntPtr ptr = IntPtr.Zero;
            if (type.IsSubclassOf(typeof(MonoBehaviour)))
            {
                ptr = GetScript(scene, GetInstanceID(), name_space, type.Name);
            }
            else
            {
                ptr = GetComponentFromScript(scene, GetInstanceID(), name_space, type.Name);
            }

            if (ptr == IntPtr.Zero)
                return null;
            return GCHandle.FromIntPtr(ptr).Target as T;
        }

        [DllImport("__Internal")] private static extern object[] GetScriptsInChildren(UInt32 SceneID, UInt64 uuid, string name_space, string name, bool includeSelf);
        [DllImport("__Internal")] private static extern object[] GetComponentsInChildrenFromScript(UInt32 SceneID, UInt64 uuid, string name_space, string name, bool includeSelf);

        public Component[] GetComponentsInChildren(Type type, bool includeSelf = false)
        {
            string name_space = "";
            if (type.Namespace != null)
                name_space = type.Namespace;

            if (type.IsSubclassOf(typeof(MonoBehaviour)))
            {
                return GetScriptsInChildren(scene, GetInstanceID(), name_space, type.Name, includeSelf) as Component[];
            }
            else
            {
                return GetComponentsInChildrenFromScript(scene, GetInstanceID(), name_space, type.Name, includeSelf) as Component[];
            }
        }

        public T[] GetComponentsInChildren<T>(bool includeSelf = false) where T : Component
        {
            Type type = typeof(T);
            string name_space = "";
            if (type.Namespace != null)
                name_space = type.Namespace;

            if (type.IsSubclassOf(typeof(MonoBehaviour)))
            {
                return GetScriptsInChildren(scene, GetInstanceID(), name_space, type.Name, includeSelf) as T[];
            }
            else
            {
                return GetComponentsInChildrenFromScript(scene, GetInstanceID(), name_space, type.Name, includeSelf) as T[];
            }
        }

        [DllImport("__Internal")] private static extern void RemoveScript(UInt32 SceneID, UInt64 uuid, string name_space, string name);
        [DllImport("__Internal")] private static extern void RemoveComponentFromScript(UInt32 SceneID, UInt64 uuid, string name_space, string name);

        public void RemoveComponent(Type type)
        {
            if (type == typeof(Transform))
            {
                Debug.LogError("RemoveComponent failed: Cannot destroy transform!");
                return;
            }
            //else if (type == typeof(Collider2D))
            //{
            //    Debug.LogError("RemoveComponent failed: Cannot destroy Collider2D! Remove BoxCollider2D/CircleCollider2D instead");
            //    return;
            //}

            string name_space = "";
            if (type.Namespace != null)
                name_space = type.Namespace;

            if (type.IsSubclassOf(typeof(MonoBehaviour)))
            {
                RemoveScript(scene, GetInstanceID(), name_space, type.Name);
            }
            else
            {
                RemoveComponentFromScript(scene, GetInstanceID(), name_space, type.Name);
            }
        }

        public void RemoveComponent<T>() where T : Component
        {
            Type type = typeof(T);

            if (type == typeof(Transform))
            {
                Debug.LogError("RemoveComponent<T> failed: Cannot destroy transform!");
                return;
            }
            //else if (type == typeof(Collider2D))
            //{
            //    Debug.LogError("RemoveComponent failed: Cannot destroy Collider2D! Remove BoxCollider2D/CircleCollider2D instead");
            //    return;
            //}

            string name_space = "";
            if (type.Namespace != null)
                name_space = type.Namespace;

            if (type.IsSubclassOf(typeof(MonoBehaviour)))
            {
                RemoveScript(scene, GetInstanceID(), name_space, type.Name);
            }
            else
            {
                RemoveComponentFromScript(scene, GetInstanceID(), name_space, type.Name);
            }
        }

        #endregion Script/Component
    }
}