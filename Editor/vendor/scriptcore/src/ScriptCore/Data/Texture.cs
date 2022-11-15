namespace Ouroboros
{
    public class Texture : Asset
    {
        public Texture(ulong assetID) : base(assetID)
        {
            if (type != Type.Texture)
                throw new System.ArgumentException("given asset ID (" + assetID + ") is not a texture");
        }

        public Texture(string path) : base(path)
        {
            if (type != Type.Texture)
                throw new System.ArgumentException("given path (" + path + ") is not a texture");
        }

        public static Texture[] GetAll()
        {
            return GetByType<Texture>(Type.Texture);
        }
    }
}