using System.Runtime.InteropServices;

namespace Ouroboros
{
    public class AudioClip : Asset
    {
        [DllImport("__Internal")] private static extern short AudioClip_GetSoundID(ulong assetID);

        private short soundID;

        public AudioClip(ulong assetID) : base(assetID)
        {
            if (type != Type.Audio)
                throw new System.ArgumentException("given asset ID (" + assetID + ") is not an audio clip");
            soundID = AudioClip_GetSoundID(id);
        }

        public AudioClip(string path) : base(path)
        {
            if (type != Type.Audio)
                throw new System.ArgumentException("given path (" + path + ") is not an audio clip");
            soundID = AudioClip_GetSoundID(id);
        }

        [DllImport("__Internal")] private static extern float AudioClip_GetLength(short id);

        public float length
        {
            get { return AudioClip_GetLength(soundID); }
        }

        [DllImport("__Internal")] private static extern uint AudioClip_GetSampleCount(short id);

        public float samples
        {
            get { return AudioClip_GetSampleCount(soundID); }
        }

        public static AudioClip[] GetAll()
        {
            return GetByType<AudioClip>(Type.Audio);
        }
    }
}