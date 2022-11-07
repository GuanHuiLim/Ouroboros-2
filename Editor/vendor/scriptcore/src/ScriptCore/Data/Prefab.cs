using System.Runtime.InteropServices;

namespace Ouroboros
{
    public struct Prefab
    {
        private string filePath;

        public Prefab(string filePath)
        {
            this.filePath = filePath;
        }

        [DllImport("__Internal")] private static extern System.IntPtr InstantiatePrefab(uint sceneID, string filePath, ulong parentID);

        public GameObject Instantiate()
        {
            System.IntPtr ptr = InstantiatePrefab(SceneManager.GetActiveScene(), filePath, 0);
            if (ptr == System.IntPtr.Zero)
                return null;
            return GCHandle.FromIntPtr(ptr).Target as GameObject;
        }

        public GameObject Instantiate(Transform parent)
        {
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