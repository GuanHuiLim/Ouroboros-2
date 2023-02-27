using System.Runtime.InteropServices;

namespace Ouroboros
{
    public class RectTransform : Component
    {
        [DllImport("__Internal")] private static extern Vector3 RectTransformComponent_GetAnchoredPosition(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void RectTransformComponent_SetAnchoredPosition(uint sceneID, ulong instanceID, Vector3 value);

        public Vector3 anchoredPosition
        {
            get { return RectTransformComponent_GetAnchoredPosition(gameObject.scene, gameObject.GetInstanceID()); }
            set { RectTransformComponent_SetAnchoredPosition(gameObject.scene, gameObject.GetInstanceID(), value); }
        }

        [DllImport("__Internal")] private static extern Vector3 RectTransformComponent_GetLocalEulerAngles(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void RectTransformComponent_SetLocalEulerAngles(uint sceneID, ulong instanceID, Vector3 value);

        public Vector3 localEulerAngles
        {
            get { return RectTransformComponent_GetLocalEulerAngles(gameObject.scene, gameObject.GetInstanceID()); }
            set { RectTransformComponent_SetLocalEulerAngles(gameObject.scene, gameObject.GetInstanceID(), value); }
        }

        [DllImport("__Internal")] private static extern Vector3 RectTransformComponent_GetLocalScale(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void RectTransformComponent_SetLocalScale(uint sceneID, ulong instanceID, Vector3 value);

        public Vector3 localScale
        {
            get { return RectTransformComponent_GetLocalScale(gameObject.scene, gameObject.GetInstanceID()); }
            set { RectTransformComponent_SetLocalScale(gameObject.scene, gameObject.GetInstanceID(), value); }
        }

        [DllImport("__Internal")] private static extern Vector2 RectTransformComponent_GetSize(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void RectTransformComponent_SetSize(uint sceneID, ulong instanceID, Vector2 value);

        public Vector2 size
        {
            get { return RectTransformComponent_GetSize(gameObject.scene, gameObject.GetInstanceID()); }
            set { RectTransformComponent_SetSize(gameObject.scene, gameObject.GetInstanceID(), value); }
        }

        [DllImport("__Internal")] private static extern Vector2 RectTransformComponent_GetPivot(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void RectTransformComponent_SetPivot(uint sceneID, ulong instanceID, Vector2 value);

        public Vector2 pivot
        {
            get { return RectTransformComponent_GetPivot(gameObject.scene, gameObject.GetInstanceID()); }
            set { RectTransformComponent_SetPivot(gameObject.scene, gameObject.GetInstanceID(), value); }
        }

        [DllImport("__Internal")] private static extern Vector2 RectTransformComponent_GetAnchorMin(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void RectTransformComponent_SetAnchorMin(uint sceneID, ulong instanceID, Vector2 value);

        public Vector2 anchorMin
        {
            get { return RectTransformComponent_GetAnchorMin(gameObject.scene, gameObject.GetInstanceID()); }
            set { RectTransformComponent_SetAnchorMin(gameObject.scene, gameObject.GetInstanceID(), value); }
        }

        [DllImport("__Internal")] private static extern Vector2 RectTransformComponent_GetAnchorMax(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void RectTransformComponent_SetAnchorMax(uint sceneID, ulong instanceID, Vector2 value);

        public Vector2 anchorMax
        {
            get { return RectTransformComponent_GetAnchorMax(gameObject.scene, gameObject.GetInstanceID()); }
            set { RectTransformComponent_SetAnchorMax(gameObject.scene, gameObject.GetInstanceID(), value); }
        }
    }
}