using System;
using System.Runtime.InteropServices;

namespace Ouroboros
{
    public struct Scene
    {
        private UInt32 m_SceneID;

        public Scene(UInt32 sceneID)
        {
            m_SceneID = sceneID;
        }

        public UInt32 GetSceneID()
        {
            return m_SceneID;
        }

        [DllImport("__Internal")] private static extern IntPtr Scene_GetName(UInt32 sceneID);

        public string name
        {
            get
            {
                GCHandle stringPtr = GCHandle.FromIntPtr(Scene_GetName(GetSceneID()));
                string name = (string)stringPtr.Target;
                stringPtr.Free();
                return name;
            }
        }

        [DllImport("__Internal")] private static extern IntPtr Scene_GetPath(UInt32 sceneID);

        public string path
        {
            get
            {
                GCHandle stringPtr = GCHandle.FromIntPtr(Scene_GetPath(GetSceneID()));
                string name = (string)stringPtr.Target;
                stringPtr.Free();
                return name;
            }
        }

        [DllImport("__Internal")] private static extern bool Scene_IsLoaded(UInt32 sceneID);

        public bool isLoaded
        {
            get { return Scene_IsLoaded(GetSceneID()); }
        }

        [DllImport("__Internal")] private static extern bool Scene_IsValid(UInt32 sceneID);

        public bool IsValid()
        {
            return Scene_IsValid(GetSceneID());
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
            return "Scene(" + m_SceneID + ")";
        }

        public static bool operator!=(Scene lhs, Scene rhs)
        {
            return lhs.m_SceneID != rhs.m_SceneID;
        }

        public static bool operator==(Scene lhs, Scene rhs)
        {
            return lhs.m_SceneID == rhs.m_SceneID;
        }

        public static implicit operator UInt32(Scene scene)
        {
            return scene.GetSceneID();
        }
    }
}