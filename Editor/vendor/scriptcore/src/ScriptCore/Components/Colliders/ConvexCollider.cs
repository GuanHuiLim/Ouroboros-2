using System;
using System.Runtime.InteropServices;

namespace Ouroboros
{
    public class ConvexCollider : Collider
    {
        //[DllImport("__Internal")] private static extern Vector3[] ConvexCollider_GetVertices(UInt32 sceneID, UInt64 uuid);

        //public Vector3[] vertices
        //{
        //    get { return ConvexCollider_GetVertices(gameObject.scene, gameObject.GetInstanceID()); }
        //}

        [DllImport("__Internal")] private static extern Vector3[] ConvexCollider_GetWorldVertices(UInt32 sceneID, UInt64 uuid);

        public Vector3[] worldVertices
        {
            get { return ConvexCollider_GetWorldVertices(gameObject.scene, gameObject.GetInstanceID()); }
        }
    }
}