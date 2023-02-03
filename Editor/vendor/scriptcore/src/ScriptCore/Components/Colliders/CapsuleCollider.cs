using System;
using System.Runtime.InteropServices;

namespace Ouroboros
{
    public class CapsuleCollider : Collider
    {
        [DllImport("__Internal")] private static extern float CapsuleColliderComponent_GetRadius(UInt32 sceneID, UInt64 uuid);
        [DllImport("__Internal")] private static extern void CapsuleColliderComponent_SetRadius(UInt32 sceneID, UInt64 uuid, float radius);

        public float radius
        {
            get { return CapsuleColliderComponent_GetRadius(gameObject.scene, gameObject.GetInstanceID()); }
            set { CapsuleColliderComponent_SetRadius(gameObject.scene, gameObject.GetInstanceID(), value); }
        }

        [DllImport("__Internal")] private static extern float CapsuleColliderComponent_GetHalfHeight(UInt32 sceneID, UInt64 uuid);
        [DllImport("__Internal")] private static extern void CapsuleColliderComponent_SetHalfHeight(UInt32 sceneID, UInt64 uuid, float halfHeight);

        public float halfHeight
        {
            get { return CapsuleColliderComponent_GetHalfHeight(gameObject.scene, gameObject.GetInstanceID()); }
            set { CapsuleColliderComponent_SetHalfHeight(gameObject.scene, gameObject.GetInstanceID(), value); }
        }
    }
}