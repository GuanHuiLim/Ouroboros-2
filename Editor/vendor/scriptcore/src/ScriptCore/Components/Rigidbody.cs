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

        [DllImport("__Internal")] private static extern bool RigidbodyComponent_GetGravity(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void RigidbodyComponent_SetGravity(uint sceneID, ulong instanceID, bool value);

        public bool useGravity
        {
            get { return RigidbodyComponent_GetGravity(gameObject.scene, gameObject.GetInstanceID()); }
            set { RigidbodyComponent_SetGravity(gameObject.scene, gameObject.GetInstanceID(), value); }
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

        [DllImport("__Internal")] private static extern bool RigidbodyComponent_GetLockXAxisPosition(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void RigidbodyComponent_SetLockXAxisPosition(uint sceneID, ulong instanceID, bool value);

        [DllImport("__Internal")] private static extern bool RigidbodyComponent_GetLockYAxisPosition(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void RigidbodyComponent_SetLockYAxisPosition(uint sceneID, ulong instanceID, bool value);

        [DllImport("__Internal")] private static extern bool RigidbodyComponent_GetLockZAxisPosition(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void RigidbodyComponent_SetLockZAxisPosition(uint sceneID, ulong instanceID, bool value);

        [DllImport("__Internal")] private static extern bool RigidbodyComponent_GetLockXAxisRotation(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void RigidbodyComponent_SetLockXAxisRotation(uint sceneID, ulong instanceID, bool value);

        [DllImport("__Internal")] private static extern bool RigidbodyComponent_GetLockYAxisRotation(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void RigidbodyComponent_SetLockYAxisRotation(uint sceneID, ulong instanceID, bool value);

        [DllImport("__Internal")] private static extern bool RigidbodyComponent_GetLockZAxisRotation(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void RigidbodyComponent_SetLockZAxisRotation(uint sceneID, ulong instanceID, bool value);

        public RigidbodyConstraints constraints
        {
            get { return GetConstraints(); }
            set { SetConstraints(value); }
        }

        private RigidbodyConstraints GetConstraints()
        {
            RigidbodyConstraints value = RigidbodyConstraints.None;

            if (RigidbodyComponent_GetLockXAxisPosition(gameObject.scene, gameObject.GetInstanceID()))
                value |= RigidbodyConstraints.FreezePositionX;
            if (RigidbodyComponent_GetLockYAxisPosition(gameObject.scene, gameObject.GetInstanceID()))
                value |= RigidbodyConstraints.FreezePositionY;
            if (RigidbodyComponent_GetLockZAxisPosition(gameObject.scene, gameObject.GetInstanceID()))
                value |= RigidbodyConstraints.FreezePositionZ;

            if (RigidbodyComponent_GetLockXAxisRotation(gameObject.scene, gameObject.GetInstanceID()))
                value |= RigidbodyConstraints.FreezeRotationX;
            if (RigidbodyComponent_GetLockYAxisRotation(gameObject.scene, gameObject.GetInstanceID()))
                value |= RigidbodyConstraints.FreezeRotationY;
            if (RigidbodyComponent_GetLockZAxisRotation(gameObject.scene, gameObject.GetInstanceID()))
                value |= RigidbodyConstraints.FreezeRotationZ;

            return value;
        }

        private void SetConstraints(RigidbodyConstraints value)
        {
            RigidbodyComponent_SetLockXAxisPosition(gameObject.scene, gameObject.GetInstanceID(), (value & RigidbodyConstraints.FreezePositionX) != RigidbodyConstraints.None);
            RigidbodyComponent_SetLockYAxisPosition(gameObject.scene, gameObject.GetInstanceID(), (value & RigidbodyConstraints.FreezePositionY) != RigidbodyConstraints.None);
            RigidbodyComponent_SetLockZAxisPosition(gameObject.scene, gameObject.GetInstanceID(), (value & RigidbodyConstraints.FreezePositionZ) != RigidbodyConstraints.None);

            RigidbodyComponent_SetLockXAxisRotation(gameObject.scene, gameObject.GetInstanceID(), (value & RigidbodyConstraints.FreezeRotationX) != RigidbodyConstraints.None);
            RigidbodyComponent_SetLockYAxisRotation(gameObject.scene, gameObject.GetInstanceID(), (value & RigidbodyConstraints.FreezeRotationY) != RigidbodyConstraints.None);
            RigidbodyComponent_SetLockZAxisRotation(gameObject.scene, gameObject.GetInstanceID(), (value & RigidbodyConstraints.FreezeRotationZ) != RigidbodyConstraints.None);
        }
    }

    public enum RigidbodyConstraints
    {
        None = 0,               // 0000 0000
        FreezePositionX = 2,    // 0000 0010
        FreezePositionY = 4,    // 0000 0100
        FreezePositionZ = 8,    // 0000 1000
        FreezePosition = 14,    // 0000 1110
        FreezeRotationX = 16,   // 0001 0000
        FreezeRotationY = 32,   // 0010 0000
        FreezeRotationZ = 64,   // 0100 0000
        FreezeRotation = 112,   // 0111 0000
        FreezeAll = 126         // 0111 1110
    }
}