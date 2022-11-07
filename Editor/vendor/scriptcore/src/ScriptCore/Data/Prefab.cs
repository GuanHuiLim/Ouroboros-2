using System.Runtime.InteropServices;

namespace Ouroboros
{
    public class Prefab
    {
        private string filePath;

        public Prefab(string filePath)
        {
            this.filePath = filePath;
        }

        [DllImport("__Internal")] private static extern bool CheckValidPrefabPath(string filePath);

        public bool IsValid()
        {
            return CheckValidPrefabPath(filePath);
        }

        [DllImport("__Internal")] private static extern System.IntPtr InstantiatePrefab(uint sceneID, string filePath, ulong parentID);

        public GameObject Instantiate()
        {
            if(!IsValid())
                throw new System.Exception("Prefab.Instantiate Invalid File Path: " + filePath);

            System.IntPtr ptr = InstantiatePrefab(SceneManager.GetActiveScene(), filePath, 0);
            if (ptr == System.IntPtr.Zero)
                return null;
            return GCHandle.FromIntPtr(ptr).Target as GameObject;
        }

        public GameObject Instantiate(Transform parent)
        {
            if (!IsValid())
                throw new System.Exception("Prefab.Instantiate Invalid File Path: " + filePath);
            
            System.IntPtr ptr = InstantiatePrefab(SceneManager.GetActiveScene(), filePath, parent.gameObject.GetInstanceID());
            if (ptr == System.IntPtr.Zero)
                return null;
            return GCHandle.FromIntPtr(ptr).Target as GameObject;
        }

        public override string ToString()
        {
            return "Prefab(\"" + filePath + "\")";
        }
    }
}