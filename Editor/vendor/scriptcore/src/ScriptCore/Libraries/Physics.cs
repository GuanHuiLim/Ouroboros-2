using System.Runtime.InteropServices;

namespace Ouroboros
{
    public static class Physics
    {
        [DllImport("__Internal")] private static extern bool Physics_RaycastBasic(Vector3 origin, Vector3 dir);
        [DllImport("__Internal")] private static extern bool Physics_Raycast(Vector3 origin, Vector3 dir, float maxDistance);

        public static bool Raycast(Vector3 origin, Vector3 direction)
        {
            return Physics_RaycastBasic(origin, direction);
        }

        public static bool Raycast(Vector3 origin, Vector3 direction, float maxDistance)
        {
            return Physics_Raycast(origin, direction, maxDistance);
        }

        [DllImport("__Internal")] private static extern System.IntPtr Physics_RaycastBasic_WithData(Vector3 origin, Vector3 dir);
        [DllImport("__Internal")] private static extern System.IntPtr Physics_Raycast_WithData(Vector3 origin, Vector3 dir, float maxDistance);

        public static bool Raycast(Vector3 origin, Vector3 direction, out RaycastHit hitInfo)
        {
            hitInfo = new RaycastHit();
            System.IntPtr ptr = Physics_RaycastBasic_WithData(origin, direction);
            if (ptr == System.IntPtr.Zero)
                return false;
            GCHandle result = GCHandle.FromIntPtr(ptr);
            hitInfo = (RaycastHit)result.Target;
            result.Free();
            return true;
        }

        public static bool Raycast(Vector3 origin, Vector3 direction, out RaycastHit hitInfo, float maxDistance)
        {
            hitInfo = new RaycastHit();
            System.IntPtr ptr = Physics_Raycast_WithData(origin, direction, maxDistance);
            if (ptr == System.IntPtr.Zero)
                return false;
            GCHandle result = GCHandle.FromIntPtr(ptr);
            hitInfo = (RaycastHit)result.Target;
            result.Free();
            return true;
        }

        [DllImport("__Internal")] private static extern RaycastHit[] Physics_RaycastAllBasic(Vector3 origin, Vector3 dir);
        [DllImport("__Internal")] private static extern RaycastHit[] Physics_RaycastAll(Vector3 origin, Vector3 dir, float maxDistance);

        public static RaycastHit[] RaycastAll(Vector3 origin, Vector3 direction)
        {
            return Physics_RaycastAllBasic(origin, direction);
        }

        public static RaycastHit[] RaycastAll(Vector3 origin, Vector3 direction, float maxDistance)
        {
            return Physics_RaycastAll(origin, direction, maxDistance);
        }
    }
}