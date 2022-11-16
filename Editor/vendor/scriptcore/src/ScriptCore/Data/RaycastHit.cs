using System.Runtime.InteropServices;

namespace Ouroboros
{
    [StructLayout(LayoutKind.Sequential)]
    public struct RaycastHit
    {
        public Transform transform;
        public Rigidbody rigidbody;
        public Collider collider;

        public Vector3 point;
        public Vector3 normal;
        public float distance;
    }
}