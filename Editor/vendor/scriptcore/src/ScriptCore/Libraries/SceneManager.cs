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
    }
}