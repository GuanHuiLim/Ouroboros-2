using System.Runtime.InteropServices;

namespace Ouroboros
{
    public class AudioClip : Asset
    {
        public AudioClip(ulong assetID) : base(assetID)
        {
            if (type != Type.Audio)
                throw new System.ArgumentException("given asset ID (" + assetID + ") is not an audio clip");
        }

        public AudioClip(string path) : base(path)
        {
            if (type != Type.Audio)
                throw new System.ArgumentException("given path (" + path + ") is not an audio clip");
        }
    }
}