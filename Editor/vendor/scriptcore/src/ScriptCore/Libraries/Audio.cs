using System.Runtime.InteropServices;

namespace Ouroboros
{
    public enum AudioSourceGroup : int
    {
        None = 0,
        SFXGeneral,
        SFXVoice,
        SFXEnvironment,
        Music,
        _MAX,
    }

    public static class Audio
    {
        [DllImport("__Internal")] private static extern float Audio_GetGroupVolume(int group);
        [DllImport("__Internal")] private static extern void Audio_SetGroupVolume(int group, float value);

        public static float GetGroupVolume(AudioSourceGroup group)
        {
            return Audio_GetGroupVolume((int)group);
        }

        public static void SetGroupVolume(AudioSourceGroup group, float value)
        {
            Audio_SetGroupVolume((int)group, value);
        }
    }
}