using System;
using System.Runtime.InteropServices;

namespace Ouroboros
{
    public static class SceneManager
    {
        [DllImport("__Internal")] private static extern UInt32 SceneManager_GetActiveScene();

        public static Scene GetActiveScene()
        {
            return new Scene(SceneManager_GetActiveScene());
        }

        [DllImport("__Internal")] private static extern bool SceneManager_LoadSceneByIndex(UInt32 sceneID);
        public static bool LoadScene(Scene scene)
        {
            return SceneManager_LoadSceneByIndex(scene.GetID());
        }
        public static bool LoadScene(UInt32 sceneID)
        {
            return SceneManager_LoadSceneByIndex(sceneID);
        }

        [DllImport("__Internal")] private static extern bool SceneManager_LoadSceneByName(string sceneName);
        public static bool LoadScene(string sceneName)
        {
            return SceneManager_LoadSceneByName(sceneName);
        }

        [DllImport("__Internal")] private static extern IntPtr SceneManager_PreloadSceneByIndex(UInt32 sceneID);
        [DllImport("__Internal")] private static extern IntPtr SceneManager_PreloadSceneByName(string sceneName);

        public static LoadProgress PreloadScene(UInt32 sceneID)
        {
            IntPtr returnPtr = SceneManager_PreloadSceneByIndex(sceneID);
            if (returnPtr == IntPtr.Zero)
                return null;
            GCHandle handle = GCHandle.FromIntPtr(returnPtr);
            LoadProgress progress = handle.Target as LoadProgress;
            return progress;
        }
        public static LoadProgress PreloadScene(string sceneName)
        {
            IntPtr returnPtr = SceneManager_PreloadSceneByName(sceneName);
            if (returnPtr == IntPtr.Zero)
                return null;
            GCHandle handle = GCHandle.FromIntPtr(returnPtr);
            LoadProgress progress = handle.Target as LoadProgress;
            return progress;
        }
}
}