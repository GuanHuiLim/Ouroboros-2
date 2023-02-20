using System;
using System.Runtime.InteropServices;

namespace Ouroboros
{
    public class BoxCollider : Collider
    {
        [DllImport("__Internal")] private static extern void BoxCollider_GetSize(UInt32 sceneID, UInt64 uuid, out float x, out float y, out float z);
        [DllImport("__Internal")] private static extern void BoxCollider_SetSize(UInt32 sceneID, UInt64 uuid, float x, float y, float z);

        public Vector3 size
        {
            get
            {
                float x, y, z;
                BoxCollider_GetSize(gameObject.scene, gameObject.GetInstanceID(), out x, out y, out z);
                return new Vector3(x, y, z);
            }
            set { BoxCollider_SetSize(gameObject.scene, gameObject.GetInstanceID(), value.x, value.y, value.z); }
        }


        [DllImport("__Internal")] private static extern void BoxCollider_GetHalfExtents(UInt32 sceneID, UInt64 uuid, out float x, out float y, out float z);
        [DllImport("__Internal")] private static extern void BoxCollider_SetHalfExtents(UInt32 sceneID, UInt64 uuid, float x, float y, float z);

        public Vector3 halfExtents
        {
            get
            {
                float x, y, z;
                BoxCollider_GetHalfExtents(gameObject.scene, gameObject.GetInstanceID(), out x, out y, out z);
                return new Vector3(x, y, z);
            }
            set { BoxCollider_SetHalfExtents(gameObject.scene, gameObject.GetInstanceID(), value.x, value.y, value.z); }
        }

        [DllImport("__Internal")] private static extern void BoxCollider_GetGlobalHalfExtents(UInt32 sceneID, UInt64 uuid, out float x, out float y, out float z);

        public Vector3 globalHalfExtents
        {
            get
            {
                float x, y, z;
                BoxCollider_GetGlobalHalfExtents(gameObject.scene, gameObject.GetInstanceID(), out x, out y, out z);
                return new Vector3(x, y, z);
            }
        }
    }
}