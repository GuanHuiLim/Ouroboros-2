using System;
using System.Runtime.InteropServices;

namespace Ouroboros
{
    public class Object
    {
        [DllImport("__Internal")] private static extern IntPtr DuplicateGameObject(uint sceneID, ulong uuid);

        public static GameObject Instantiate(GameObject original)
        {
            System.IntPtr ptr = DuplicateGameObject(SceneManager.GetActiveScene(), original.GetInstanceID());
            if (ptr == System.IntPtr.Zero)
                return null;
            return GCHandle.FromIntPtr(ptr).Target as GameObject;
        }

        public static GameObject Instantiate(GameObject original, Transform parent)
        {
            System.IntPtr ptr = DuplicateGameObject(SceneManager.GetActiveScene(), original.GetInstanceID());
            if (ptr == System.IntPtr.Zero)
                return null;
            GameObject instance = GCHandle.FromIntPtr(ptr).Target as GameObject;
            instance.transform.SetParent(parent);
            return instance;
        }

        public static GameObject Instantiate(Prefab original)
        {
            return original.Instantiate();
        }

        public static GameObject Instantiate(Prefab original, Transform parent)
        {
            return original.Instantiate(parent);
        }

        [DllImport("__Internal")] private static extern void DestroyEntity(UInt32 sceneID, UInt64 uuid);
        [DllImport("__Internal")] private static extern void RemoveScript(UInt32 sceneID, UInt64 uuid, string name_space, string name);
        [DllImport("__Internal")] private static extern void RemoveComponentFromScript(UInt32 sceneID, UInt64 uuid, string name_space, string name);

        public static void Destroy(Object obj)
        {
            if (obj == null)
            {
                throw new NullReferenceException();
                //Debug.LogError("Destroying failed: Cannot destroy null Object!");
                //return;
            }
            Type type = obj.GetType();
            if (type == typeof(Transform))
            {
                Debug.LogError("Destroying failed: Cannot destroy transform!");
                return;
            }
            //else if (type == typeof(Collider2D))
            //{
            //    Debug.LogError("Destroying failed: Cannot destroy Collider2D! Remove BoxCollider2D/CircleCollider2D instead");
            //    return;
            //}
            else if (type == typeof(GameObject) || type.IsSubclassOf(typeof(GameObject))) // GameObject
            {
                GameObject gameObject = obj as GameObject;
                DestroyEntity(gameObject.scene, gameObject.GetInstanceID());
            }
            else if (type == typeof(MonoBehaviour) || type.IsSubclassOf(typeof(MonoBehaviour))) // MonoBehaviour (C# Scripts)
            {
                MonoBehaviour script = obj as MonoBehaviour;
                string name_space = "";
                if (type.Namespace != null)
                    name_space = type.Namespace;
                RemoveScript(script.gameObject.scene, script.gameObject.GetInstanceID(), name_space, type.Name);
            }
            else if (type == typeof(Component) || type.IsSubclassOf(typeof(Component))) // C++ Components
            {
                Component component = obj as Component;
                RemoveComponentFromScript(component.gameObject.scene, component.gameObject.GetInstanceID(), type.Namespace ?? "", type.Name);
            }
        }
    }
}
