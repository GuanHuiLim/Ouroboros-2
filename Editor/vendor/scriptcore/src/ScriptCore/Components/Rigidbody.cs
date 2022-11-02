using System.Runtime.InteropServices;

namespace Ouroboros
{
    public class Rigidbody : Component
    {
        [DllImport("__Internal")] private static extern bool RigidbodyComponent_GetIsStatic(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void RigidbodyComponent_SetIsStatic(uint sceneID, ulong instanceID, bool value);

        public bool isStatic
        {
            get { return RigidbodyComponent_GetIsStatic(gameObject.scene, gameObject.GetInstanceID()); }
            set { RigidbodyComponent_SetIsStatic(gameObject.scene, gameObject.GetInstanceID(), value); }
        }

        [DllImport("__Internal")] private static extern bool RigidbodyComponent_GetIsTrigger(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void RigidbodyComponent_SetIsTrigger(uint sceneID, ulong instanceID, bool value);

        public bool isTrigger
        {
            get { return RigidbodyComponent_GetIsTrigger(gameObject.scene, gameObject.GetInstanceID()); }
            set { RigidbodyComponent_SetIsTrigger(gameObject.scene, gameObject.GetInstanceID(), value); }
        }

        [DllImport("__Internal")] private static extern bool RigidbodyComponent_GetGravityDisabled(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void RigidbodyComponent_SetGravityDisabled(uint sceneID, ulong instanceID, bool value);

        public bool gravityDisabled
        {
            get { return RigidbodyComponent_GetGravityDisabled(gameObject.scene, gameObject.GetInstanceID()); }
            set { RigidbodyComponent_SetGravityDisabled(gameObject.scene, gameObject.GetInstanceID(), value); }
        }

        [DllImport("__Internal")] private static extern void Rigidbody_GetVelocity(uint sceneID, ulong instanceID, out float x, out float y, out float z);
        [DllImport("__Internal")] private static extern void Rigidbody_SetVelocity(uint sceneID, ulong uuid, float x, float y, float z);

        public Vector3 velocity
        {
            get 
            {
                float x, y, z;
                Rigidbody_GetVelocity(gameObject.scene, gameObject.GetInstanceID(), out x, out y, out z);
                return new Vector3(x, y, z);
            }
            set { Rigidbody_SetVelocity(gameObject.scene, gameObject.GetInstanceID(), value.x, value.y, value.z); }
        }

        [DllImport("__Internal")] private static extern void Rigidbody_AddForce(uint sceneID, ulong uuid, float x, float y, float z);

        public void AddForce(Vector3 force)
        {
            Rigidbody_AddForce(gameObject.scene, gameObject.GetInstanceID(), force.x, force.y, force.z);
        }
    }
}