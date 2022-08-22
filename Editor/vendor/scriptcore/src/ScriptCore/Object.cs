//using System;
//using System.Runtime.InteropServices;

//namespace Ouroboros
//{
//    public class Object
//    {
//        [DllImport("__Internal")] private static extern IntPtr InstantiateEntity(int src);

//        public static GameObject Instantiate(GameObject original, Transform parent)
//        {
//            IntPtr ptr = InstantiateEntity(original.GetInstanceID());
//            GameObject instance = GCHandle.FromIntPtr(ptr).Target as GameObject;
//            instance.transform.SetParent(parent);
//            return instance;
//        }

//        [DllImport("__Internal")] private static extern void DestroyEntity(int id);
//        [DllImport("__Internal")] private static extern void RemoveScript(int id, string name_space, string name);
//        [DllImport("__Internal")] private static extern void RemoveComponentFromScript(int id, string name_space, string name);

//        public static void Destroy(Object obj)
//        {
//            if (obj == null)
//            {
//                Debug.LogError("Destroying failed: Cannot destroy null Object!");
//                return;
//            }
//            Type type = obj.GetType();
//            if(type == typeof(Transform))
//            {
//                Debug.LogError("Destroying failed: Cannot destroy transform!");
//                return;
//            }
//            else if (type == typeof(Collider2D))
//            {
//                Debug.LogError("Destroying failed: Cannot destroy Collider2D! Remove BoxCollider2D/CircleCollider2D instead");
//                return;
//            }
//            else if(type == typeof(GameObject) || type.IsSubclassOf(typeof(GameObject))) // GameObject
//            {
//                GameObject gameObject = obj as GameObject;
//                DestroyEntity(gameObject.GetInstanceID());
//            }
//            else if (type == typeof(MonoBehaviour) || type.IsSubclassOf(typeof(MonoBehaviour))) // MonoBehaviour (C# Scripts)
//            {
//                MonoBehaviour script = obj as MonoBehaviour;
//                string name_space = "";
//                if (type.Namespace != null)
//                    name_space = type.Namespace;
//                RemoveScript(script.gameObject.GetInstanceID(), name_space, type.Name);
//            }
//            else if(type == typeof(Component) || type.IsSubclassOf(typeof(Component))) // C++ Components
//            {
//                Component component = obj as Component;
//                RemoveComponentFromScript(component.gameObject.GetInstanceID(), type.Namespace ?? "", type.Name);
//            }
//        }
//    }
//}
