using System.Runtime.InteropServices;

namespace Ouroboros
{
    public class Rigidbody : Component
    {
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