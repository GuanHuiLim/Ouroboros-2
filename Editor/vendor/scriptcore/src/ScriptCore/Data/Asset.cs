using System.Runtime.InteropServices;

namespace Ouroboros
{
    public class Asset
    {
        public enum Type
        {
            Text = 0,
            Texture,
            Font,
            Audio,
            Model,
        };

        protected ulong id = 0;

        public Asset(ulong assetID)
        {
            id = assetID;
        }

        public Asset(string path)
        {
            id = Asset_LoadAssetAtPath(path);
        }

        public ulong GetID()
        {
            return id;
        }

        [DllImport("__Internal")] protected static extern uint Asset_GetType(ulong assetID);

        public Type type
        {
            get { return (Type)Asset_GetType(id);  }
        }

        [DllImport("__Internal")] protected static extern System.IntPtr Asset_GetName(ulong assetID);

        public string name
        {
            get
            {
                GCHandle stringPtr = GCHandle.FromIntPtr(Asset_GetName(id));
                string name = (string)stringPtr.Target;
                stringPtr.Free();
                return name;
            }
        }

        public static bool operator ==(Asset lhs, Asset rhs)
        {
            if (ReferenceEquals(lhs, null) && ReferenceEquals(rhs, null)) // lhs is null, and rhs is null
                return true;
            if (!ReferenceEquals(lhs, null) && ReferenceEquals(rhs, null)) // lhs is not null, but rhs is null
                return lhs.id == 0;
            if (ReferenceEquals(lhs, null) && !ReferenceEquals(rhs, null)) // lhs is null, but rhs is not null
                return rhs.id == 0;
            return lhs.id == rhs.id;
        }

        public static bool operator !=(Asset lhs, Asset rhs)
        {
            return !(lhs == rhs);
        }

        public override bool Equals(object obj)
        {
            return base.Equals(obj);
        }

        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        public override string ToString()
        {
            return (id == 0) ? "null" : GetType() + "(" + name + ")";
        }

        [DllImport("__Internal")] protected static extern ulong Asset_LoadAssetAtPath(string path);

        public static Asset LoadAtPath(string path)
        {
            return new Asset(Asset_LoadAssetAtPath(path));
        }

        [DllImport("__Internal")] protected static extern object[] Asset_GetByType(Type assetType, string name_space, string name);

        public static T[] GetByType<T>(Type type)
        {
            string name_space = "";
            if (typeof(T).Namespace != null)
                name_space = typeof(T).Namespace;

            return Asset_GetByType(type, name_space, typeof(T).Name) as T[];
        }
    }
}