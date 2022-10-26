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
    }
}