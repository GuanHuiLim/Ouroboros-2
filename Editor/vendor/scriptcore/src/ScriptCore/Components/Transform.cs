//using System;
//using System.Numerics;
//using System.Collections.Generic;
//using System.Runtime.InteropServices;

//namespace Ouroboros
//{
//    public class Transform : Component
//    {
//        protected Transform() { }

//        //[DllImport("__Internal")] private static extern Vector3 Transform3D_GetLocalPosition(int instanceID);
//        [DllImport("__Internal")] private static extern void Transform3D_GetLocalPosition(int instanceID, out float x, out float y, out float z);
//        [DllImport("__Internal")] private static extern void Transform3D_SetLocalPosition(int instanceID, float x, float y, float z);

//        public Vector3 localPosition
//        {
//            get
//            {
//                float x, y, z;
//                Transform3D_GetLocalPosition(gameObject.GetInstanceID(), out x, out y, out z);
//                return new Vector3(x, y, z);
//            }
//            //get { return Transform3D_GetLocalPosition(gameObject.GetInstanceID()); }
//            set { Transform3D_SetLocalPosition(gameObject.GetInstanceID(), value.X, value.Y, value.Z); }
//        }

//        //[DllImport("__Internal")] private static extern Vector3 Transform3D_GetLocalPosition(int instanceID);
//        //[DllImport("__Internal")] private static extern void Transform3D_SetLocalPosition(int instanceID, Vector3 vector);

//        //public Vector3 localPosition
//        //{
//        //    get { Debug.Log(gameObject.GetInstanceID()); return Transform3D_GetLocalPosition(gameObject.GetInstanceID()); }
//        //    set { Transform3D_SetLocalPosition(gameObject.GetInstanceID(), value); }
//        //}

//        [DllImport("__Internal")] private static extern void Transform3D_GetGlobalPosition(int instanceID, out float x, out float y, out float z);
//        [DllImport("__Internal")] private static extern void Transform3D_SetGlobalPosition(int instanceID, float x, float y, float z);

//        public Vector3 position
//        {
//            get
//            {
//                float x, y, z;
//                Transform3D_GetGlobalPosition(gameObject.GetInstanceID(), out x, out y, out z);
//                return new Vector3(x, y, z);
//            }
//            set { Transform3D_SetGlobalPosition(gameObject.GetInstanceID(), value.X, value.Y, value.Z); }
//        }

//        [DllImport("__Internal")] private static extern float Transform3D_GetLocalAngle(int instanceID);
//        [DllImport("__Internal")] private static extern void Transform3D_SetLocalAngle(int instanceID, float angle);

//        public float localAngle
//        {
//            get { return Transform3D_GetLocalAngle(gameObject.GetInstanceID()); }
//            set { Transform3D_SetLocalAngle(gameObject.GetInstanceID(), value); }
//        }
//        [DllImport("__Internal")] private static extern float Transform3D_GetGlobalAngle(int instanceID);
//        //[DllImport("__Internal")] private static extern void Transform_SetGlobalAngle(int instanceID, float angle);

//        public float rotationAngle
//        {
//            get { return Transform3D_GetGlobalAngle(gameObject.GetInstanceID()); }
//            //set { Transform_SetGlobalAngle(gameObject.GetInstanceID(), value); }
//        }

//        [DllImport("__Internal")] private static extern void Transform3D_GetLocalScale(int instanceID, out float x, out float y, out float z);
//        [DllImport("__Internal")] private static extern void Transform3D_SetLocalScale(int instanceID, float x, float y, float z);

//        public Vector3 localScale
//        {
//            get
//            {
//                float x, y, z;
//                Transform3D_GetLocalScale(gameObject.GetInstanceID(), out x, out y, out z);
//                return new Vector3(x, y, z);
//            }
//            set { Transform3D_SetLocalScale(gameObject.GetInstanceID(), value.X, value.Y, value.Z); }
//        }
//        [DllImport("__Internal")] private static extern void Transform3D_GetGlobalScale(int instanceID, out float x, out float y, out float z);
//        [DllImport("__Internal")] private static extern void Transform3D_SetGlobalScale(int instanceID, float x, float y, float z);

//        public Vector3 globalScale
//        {
//            get
//            {
//                float x, y, z;
//                Transform3D_GetGlobalScale(gameObject.GetInstanceID(), out x, out y, out z);
//                return new Vector3(x, y, z);
//            }
//            set { Transform3D_SetGlobalScale(gameObject.GetInstanceID(), value.X, value.Y, value.Z); }
//        }

//        [DllImport("__Internal")] private static extern int Transform_GetChildCount(int instanceID);

//        public int childCount
//        {
//            get { return Transform_GetChildCount(gameObject.GetInstanceID()); }
//        }

//        [DllImport("__Internal")] private static extern IntPtr Transform_GetChild(int instanceID, int childIndex);

//        public Transform GetChild(int index)
//        {
//            IntPtr child = Transform_GetChild(gameObject.GetInstanceID(), index);
//            if (child == IntPtr.Zero)
//                return null;
//            return (Transform)GCHandle.FromIntPtr(child).Target;
//        }

//        [DllImport("__Internal")] private static extern void Transform_SetParent(int instanceID, int newParent, bool preserveTransforms);
//        [DllImport("__Internal")] private static extern IntPtr Transform_GetParent(int instanceID);

//        public Transform parent
//        {
//            get
//            {
//                IntPtr ptr = Transform_GetParent(gameObject.GetInstanceID());
//                if (ptr == IntPtr.Zero)
//                    return null;
//                return (Transform)GCHandle.FromIntPtr(ptr).Target;
//            }
//            set { SetParent(value); }
//        }

//        public void SetParent(Transform parent, bool worldPositionStays = false)
//        {
//            int parentID = (parent != null) ? parent.gameObject.GetInstanceID() : -1;
//            Transform_SetParent(gameObject.GetInstanceID(), parentID, worldPositionStays);
//        }
//    }
//}
