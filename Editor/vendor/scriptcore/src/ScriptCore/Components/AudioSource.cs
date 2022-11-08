using System.Runtime.InteropServices;

namespace Ouroboros
{
    public class AudioSource : Component
    {
        [DllImport("__Internal")] private static extern void AudioSourceComponent_Play(uint sceneID, ulong instanceID);

        public void Play()
        {
            AudioSourceComponent_Play(gameObject.scene, gameObject.GetInstanceID());
        }

        [DllImport("__Internal")] private static extern void AudioSourceComponent_Stop(uint sceneID, ulong instanceID);

        public void Stop()
        {
            AudioSourceComponent_Stop(gameObject.scene, gameObject.GetInstanceID());
        }

        [DllImport("__Internal")] private static extern void AudioSourceComponent_Pause(uint sceneID, ulong instanceID);

        public void Pause()
        {
            AudioSourceComponent_Pause(gameObject.scene, gameObject.GetInstanceID());
        }

        [DllImport("__Internal")] private static extern void AudioSourceComponent_UnPause(uint sceneID, ulong instanceID);

        public void UnPause()
        {
            AudioSourceComponent_UnPause(gameObject.scene, gameObject.GetInstanceID());
        }

        [DllImport("__Internal")] private static extern bool AudioSourceComponent_GetMuted(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void AudioSourceComponent_SetMuted(uint sceneID, ulong instanceID, bool value);

        public bool mute
        {
            get { return AudioSourceComponent_GetMuted(gameObject.scene, gameObject.GetInstanceID()); }
            set { AudioSourceComponent_SetMuted(gameObject.scene, gameObject.GetInstanceID(), value); }
        }

        [DllImport("__Internal")] private static extern bool AudioSourceComponent_GetPlayOnAwake(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void AudioSourceComponent_SetPlayOnAwake(uint sceneID, ulong instanceID, bool value);

        public bool playOnAwake
        {
            get { return AudioSourceComponent_GetPlayOnAwake(gameObject.scene, gameObject.GetInstanceID()); }
            set { AudioSourceComponent_SetPlayOnAwake(gameObject.scene, gameObject.GetInstanceID(), value); }
        }

        [DllImport("__Internal")] private static extern bool AudioSourceComponent_GetLoop(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void AudioSourceComponent_SetLoop(uint sceneID, ulong instanceID, bool value);

        public bool loop
        {
            get { return AudioSourceComponent_GetLoop(gameObject.scene, gameObject.GetInstanceID()); }
            set { AudioSourceComponent_SetLoop(gameObject.scene, gameObject.GetInstanceID(), value); }
        }

        [DllImport("__Internal")] private static extern float AudioSourceComponent_GetVolume(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void AudioSourceComponent_SetVolume(uint sceneID, ulong instanceID, float value);

        public float volume
        {
            get { return AudioSourceComponent_GetVolume(gameObject.scene, gameObject.GetInstanceID()); }
            set { AudioSourceComponent_SetVolume(gameObject.scene, gameObject.GetInstanceID(), value); }
        }

        [DllImport("__Internal")] private static extern float AudioSourceComponent_GetPitch(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void AudioSourceComponent_SetPitch(uint sceneID, ulong instanceID, float value);

        public float pitch
        {
            get { return AudioSourceComponent_GetPitch(gameObject.scene, gameObject.GetInstanceID()); }
            set { AudioSourceComponent_SetPitch(gameObject.scene, gameObject.GetInstanceID(), value); }
        }

        [DllImport("__Internal")] private static extern bool AudioSourceComponent_IsPlaying(uint sceneID, ulong instanceID);

        public bool isPlaying
        {
            get { return AudioSourceComponent_IsPlaying(gameObject.scene, gameObject.GetInstanceID()); }
        }

        [DllImport("__Internal")] private static extern float AudioSourceComponent_GetPlaybackTime(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void AudioSourceComponent_SetPlaybackTime(uint sceneID, ulong instanceID, float value);

        public float time
        {
            get { return AudioSourceComponent_GetPlaybackTime(gameObject.scene, gameObject.GetInstanceID()); }
            set { AudioSourceComponent_SetPlaybackTime(gameObject.scene, gameObject.GetInstanceID(), value); }
        }

        [DllImport("__Internal")] private static extern uint AudioSourceComponent_GetPlaybackTimeSamples(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void AudioSourceComponent_SetPlaybackTimeSamples(uint sceneID, ulong instanceID, uint value);

        public uint timeSamples
        {
            get { return AudioSourceComponent_GetPlaybackTimeSamples(gameObject.scene, gameObject.GetInstanceID()); }
            set { AudioSourceComponent_SetPlaybackTimeSamples(gameObject.scene, gameObject.GetInstanceID(), value); }
        }

        [DllImport("__Internal")] private static extern ulong AudioSourceComponent_GetAudioClip(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void AudioSourceComponent_SetAudioClip(uint sceneID, ulong instanceID, ulong assetID);

        public AudioClip clip
        {
            get { return new AudioClip(AudioSourceComponent_GetAudioClip(gameObject.scene, gameObject.GetInstanceID())); }
            set { AudioSourceComponent_SetAudioClip(gameObject.scene, gameObject.GetInstanceID(), value.GetID()); }
        }
    }
}