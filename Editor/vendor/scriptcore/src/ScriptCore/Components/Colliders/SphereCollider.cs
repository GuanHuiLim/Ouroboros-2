using System;
using System.Runtime.InteropServices;

namespace Ouroboros
{
    public class SphereCollider : Collider
    {
        [DllImport("__Internal")] private static extern float SphereColliderComponent_GetRadius(UInt32 sceneID, UInt64 uuid);
        [DllImport("__Internal")] private static extern void SphereColliderComponent_SetRadius(UInt32 sceneID, UInt64 uuid, float radius);

        public float radius
        {
            get { return SphereColliderComponent_GetRadius(gameObject.scene, gameObject.GetInstanceID()); }
            set { SphereColliderComponent_SetRadius(gameObject.scene, gameObject.GetInstanceID(), value); }
        }

        [DllImport("__Internal")] private static extern float SphereColliderComponent_GetGlobalRadius(UInt32 sceneID, UInt64 uuid);

        public float globalRadius
        {
            get { return SphereColliderComponent_GetGlobalRadius(gameObject.scene, gameObject.GetInstanceID()); }
        }
    }
}