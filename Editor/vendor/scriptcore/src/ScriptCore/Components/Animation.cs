using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Ouroboros
{
    public class AnimationComponent : Component
    {

        [DllImport("__Internal")] private static extern UInt64 AnimationComponent_GetParameterID(UInt32 sceneID, UInt64 instanceID, string paramName);
        [DllImport("__Internal")] private static extern UInt32 AnimationComponent_GetParameterIndex(UInt32 sceneID, UInt64 instanceID, string paramName);
        
        public UInt64 GetParameterID(string paramName)
        {
            return AnimationComponent_GetParameterID(gameObject.scene, gameObject.GetInstanceID(), paramName);
        }

        public UInt32 GetParameterIndex(string paramName)
        {
            return AnimationComponent_GetParameterIndex(gameObject.scene, gameObject.GetInstanceID(), paramName);
        }

        /*----------------
        INT
        ----------------*/
        [DllImport("__Internal")] private static extern void AnimationComponent_SetParameterByName_int(UInt32 sceneID, UInt64 instanceID, string paramName, Int32 val);
        public void SetParameterByName_int(string paramName, Int32 val)
        {
            AnimationComponent_SetParameterByName_int(gameObject.scene, gameObject.GetInstanceID(), paramName, val);
        }

        [DllImport("__Internal")] private static extern void AnimationComponent_SetParameterByID_int(UInt32 sceneID, UInt64 instanceID, UInt64 id, Int32 val);
        public void SetParameterByID_int(UInt64 id, Int32 val)
        {
            AnimationComponent_SetParameterByID_int(gameObject.scene, gameObject.GetInstanceID(), id, val);
        }

        [DllImport("__Internal")] private static extern void AnimationComponent_SetParameterByIndex_int(UInt32 sceneID, UInt64 instanceID, UInt32 index, Int32 val);
        public void SetParameterByIndex_int(UInt32 index, Int32 val)
        {
            AnimationComponent_SetParameterByIndex_int(gameObject.scene, gameObject.GetInstanceID(), index, val);
        }

        /*----------------
        FLOAT
        ----------------*/
        [DllImport("__Internal")] private static extern void AnimationComponent_SetParameterByName_float(UInt32 sceneID, UInt64 instanceID, string paramName, float val);
        public void SetParameterByName_float(string paramName, float val)
        {
            AnimationComponent_SetParameterByName_float(gameObject.scene, gameObject.GetInstanceID(), paramName, val);
        }

        [DllImport("__Internal")] private static extern void AnimationComponent_SetParameterByID_float(UInt32 sceneID, UInt64 instanceID, UInt64 id, float val);
        public void SetParameterByID_float(UInt64 id, float val)
        {
            AnimationComponent_SetParameterByID_float(gameObject.scene, gameObject.GetInstanceID(), id, val);
        }

        [DllImport("__Internal")] private static extern void AnimationComponent_SetParameterByIndex_float(UInt32 sceneID, UInt64 instanceID, UInt32 index, float val);
        public void SetParameterByIndex_float(UInt32 index, float val)
        {
            AnimationComponent_SetParameterByIndex_float(gameObject.scene, gameObject.GetInstanceID(), index, val);
        }

        /*----------------
        BOOL
        ----------------*/
        [DllImport("__Internal")] private static extern void AnimationComponent_SetParameterByName_bool(UInt32 sceneID, UInt64 instanceID, string paramName, bool val);
        public void SetParameterByName_bool(string paramName, bool val)
        {
            AnimationComponent_SetParameterByName_bool(gameObject.scene, gameObject.GetInstanceID(), paramName, val);
        }

        [DllImport("__Internal")] private static extern void AnimationComponent_SetParameterByID_bool(UInt32 sceneID, UInt64 instanceID, UInt64 id, bool val);
        public void SetParameterByID_bool(UInt64 id, bool val)
        {
            AnimationComponent_SetParameterByID_bool(gameObject.scene, gameObject.GetInstanceID(), id, val);
        }

        [DllImport("__Internal")] private static extern void AnimationComponent_SetParameterByIndex_bool(UInt32 sceneID, UInt64 instanceID, UInt32 index, bool val);
        public void SetParameterByIndex_bool(UInt32 index, bool val)
        {
            AnimationComponent_SetParameterByIndex_bool(gameObject.scene, gameObject.GetInstanceID(), index, val);
        }
    }
}
