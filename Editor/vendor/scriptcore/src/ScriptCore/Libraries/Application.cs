using System.Runtime.InteropServices;

namespace Ouroboros
{
    public static class Application
    {
        [DllImport("__Internal")] private static extern void Application_Quit();

        public static void Quit()
        {
            Application_Quit();
        }

        [DllImport("__Internal")] private static extern System.IntPtr Application_GetAssetPath();

        public static string assetPath
        {
            get
            {
                GCHandle stringPtr = GCHandle.FromIntPtr(Application_GetAssetPath());
                string path = (string)stringPtr.Target;
                stringPtr.Free();
                return path;
            }
        }
    }
}