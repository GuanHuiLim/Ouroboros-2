using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace Ouroboros
{
    public class MonoBehaviour : Component
    {
        protected MonoBehaviour()
        {

        }

        //[DllImport("__Internal")] private static extern bool CheckEntityExists(int id);

        //public static bool operator ==(MonoBehaviour lhs, MonoBehaviour rhs)
        //{
        //    if (ReferenceEquals(lhs, null) && ReferenceEquals(rhs, null)) // both lhs and rhs is null
        //    {
        //        return true;
        //    }
        //    if (!ReferenceEquals(lhs, null) && ReferenceEquals(rhs, null)) // lhs is not null, but rhs is null
        //    {
        //        return !CheckEntityExists(lhs.gameObject.GetInstanceID()) || lhs.GetComponent(lhs.GetType()) != lhs;
        //    }
        //    if (ReferenceEquals(lhs, null) && !ReferenceEquals(rhs, null)) // lhs is null, but rhs is not null
        //    {
        //        return !CheckEntityExists(rhs.gameObject.GetInstanceID()) || rhs.GetComponent(rhs.GetType()) != rhs;
        //    }
        //    return ReferenceEquals(lhs, rhs);
        //}

        //public static bool operator !=(MonoBehaviour lhs, MonoBehaviour rhs)
        //{
        //    return !(lhs == rhs);
        //}

        public override bool Equals(object obj)
        {
            return base.Equals(obj);
        }

        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        //[DllImport("__Internal")] private static extern void SetScriptEnabled(int entityID, string name_space, string name, bool enabled);
        //[DllImport("__Internal")] private static extern bool CheckScriptEnabled(int entityID, string name_space, string name);

        //public new bool enabled
        //{
        //    get { return CheckScriptEnabled(gameObject.GetInstanceID(), GetType().Namespace ?? "", GetType().Name); ; }
        //    set { SetScriptEnabled(gameObject.GetInstanceID(), GetType().Namespace ?? "", GetType().Name, value); }
        //}
    }
}