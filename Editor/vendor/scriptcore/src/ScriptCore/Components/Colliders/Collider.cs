using System.Runtime.InteropServices;

namespace Ouroboros
{
    public class Collider : Component
    {
        [DllImport("__Internal")] private static extern bool BoxColliderComponent_GetIsTrigger(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void BoxColliderComponent_SetIsTrigger(uint sceneID, ulong instanceID, bool value);

        public bool isTrigger
        {
            get { return BoxColliderComponent_GetIsTrigger(gameObject.scene, gameObject.GetInstanceID()); }
            set { BoxColliderComponent_SetIsTrigger(gameObject.scene, gameObject.GetInstanceID(), value); }
        }
    }
}