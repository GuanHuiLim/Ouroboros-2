using System;
using System.Numerics;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Ouroboros
{
    public class Transform : Component
    {
        protected Transform() { }

        [DllImport("__Internal")] private static extern void Transform3D_GetLocalPosition(UInt32 sceneID, UInt64 instanceID, out float x, out float y, out float z);
        [DllImport("__Internal")] private static extern void Transform3D_SetLocalPosition(UInt32 sceneID, UInt64 instanceID, float x, float y, float z);

        public Vector3 localPosition
        {
            get
            {
                float x, y, z;
                Transform3D_GetLocalPosition(gameObject.scene, gameObject.GetInstanceID(), out x, out y, out z);
                return new Vector3(x, y, z);
            }
            set { Transform3D_SetLocalPosition(gameObject.scene, gameObject.GetInstanceID(), value.X, value.Y, value.Z); }
        }

        [DllImport("__Internal")] private static extern void Transform3D_GetGlobalPosition(UInt32 sceneID, UInt64 instanceID, out float x, out float y, out float z);
        [DllImport("__Internal")] private static extern void Transform3D_SetGlobalPosition(UInt32 sceneID, UInt64 instanceID, float x, float y, float z);

        public Vector3 position
        {
            get
            {
                float x, y, z;
                Transform3D_GetGlobalPosition(gameObject.scene, gameObject.GetInstanceID(), out x, out y, out z);
                return new Vector3(x, y, z);
            }
            set { Transform3D_SetGlobalPosition(gameObject.scene, gameObject.GetInstanceID(), value.X, value.Y, value.Z); }
        }

        [DllImport("__Internal")] private static extern void Transform3D_GetLocalEulerAngles(UInt32 sceneID, UInt64 instanceID, out float x, out float y, out float z);
        [DllImport("__Internal")] private static extern void Transform3D_SetLocalEulerAngles(UInt32 sceneID, UInt64 instanceID, float x, float y, float z);

        public Vector3 localEulerAngles
        {
            get
            {
                float x, y, z;
                Transform3D_GetLocalEulerAngles(gameObject.scene, gameObject.GetInstanceID(), out x, out y, out z);
                return new Vector3(x, y, z);
            }
            set { Transform3D_SetLocalEulerAngles(gameObject.scene, gameObject.GetInstanceID(), value.X, value.Y, value.Z); }
        }

        [DllImport("__Internal")] private static extern void Transform3D_GetGlobalEulerAngles(UInt32 sceneID, UInt64 instanceID, out float x, out float y, out float z);
        [DllImport("__Internal")] private static extern void Transform3D_SetGlobalEulerAngles(UInt32 sceneID, UInt64 instanceID, float x, float y, float z);

        public Vector3 eulerAngles
        {
            get
            {
                float x, y, z;
                Transform3D_GetGlobalEulerAngles(gameObject.scene, gameObject.GetInstanceID(), out x, out y, out z);
                return new Vector3(x, y, z);
            }
            set { Transform3D_SetGlobalEulerAngles(gameObject.scene, gameObject.GetInstanceID(), value.X, value.Y, value.Z); }
        }

        [DllImport("__Internal")] private static extern void Transform3D_GetLocalScale(UInt32 sceneID, UInt64 uuid, out float x, out float y, out float z);
        [DllImport("__Internal")] private static extern void Transform3D_SetLocalScale(UInt32 sceneID, UInt64 uuid, float x, float y, float z);

        public Vector3 localScale
        {
            get
            {
                float x, y, z;
                Transform3D_GetLocalScale(gameObject.scene, gameObject.GetInstanceID(), out x, out y, out z);
                return new Vector3(x, y, z);
            }
            set { Transform3D_SetLocalScale(gameObject.scene, gameObject.GetInstanceID(), value.X, value.Y, value.Z); }
        }

        [DllImport("__Internal")] private static extern void Transform3D_GetGlobalScale(UInt32 sceneID, UInt64 uuid, out float x, out float y, out float z);
        [DllImport("__Internal")] private static extern void Transform3D_SetGlobalScale(UInt32 sceneID, UInt64 uuid, float x, float y, float z);

        public Vector3 scale
        {
            get
            {
                float x, y, z;
                Transform3D_GetGlobalScale(gameObject.scene, gameObject.GetInstanceID(), out x, out y, out z);
                return new Vector3(x, y, z);
            }
            set { Transform3D_SetGlobalScale(gameObject.scene, gameObject.GetInstanceID(), value.X, value.Y, value.Z); }
        }

        [DllImport("__Internal")] private static extern int Transform_GetChildCount(UInt32 sceneID, UInt64 uuid);

        public int childCount
        {
            get { return Transform_GetChildCount(gameObject.scene, gameObject.GetInstanceID()); }
        }

        [DllImport("__Internal")] private static extern IntPtr Transform_GetChild(UInt32 sceneID, UInt64 uuid, int childIndex);

        public Transform GetChild(int index)
        {
            IntPtr child = Transform_GetChild(gameObject.scene, gameObject.GetInstanceID(), index);
            if (child == IntPtr.Zero)
                return null;
            return (Transform)GCHandle.FromIntPtr(child).Target;
        }

        [DllImport("__Internal")] private static extern void Transform_SetParent(UInt32 sceneID, UInt64 uuid, UInt64 newParent, bool preserveTransforms);
        [DllImport("__Internal")] private static extern IntPtr Transform_GetParent(UInt32 sceneID, UInt64 uuid);

        public Transform parent
        {
            get
            {
                IntPtr ptr = Transform_GetParent(gameObject.scene, gameObject.GetInstanceID());
                if (ptr == IntPtr.Zero)
                    return null;
                return (Transform)GCHandle.FromIntPtr(ptr).Target;
            }
            set { SetParent(value); }
        }

        public void SetParent(Transform parent, bool worldPositionStays = false)
        {
            UInt64 parentID = (parent != null) ? parent.gameObject.GetInstanceID() : 0;
            Transform_SetParent(gameObject.scene, gameObject.GetInstanceID(), parentID, worldPositionStays);
        }
    }
}
