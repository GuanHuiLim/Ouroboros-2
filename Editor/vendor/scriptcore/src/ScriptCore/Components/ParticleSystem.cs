using System.Runtime.InteropServices;

namespace Ouroboros
{
    public class ParticleSystem : Component
    {
        [DllImport("__Internal")] private static extern void ParticleEmitterComponent_Play(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void ParticleEmitterComponent_Stop(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void ParticleEmitterComponent_ResetSystem(uint sceneID, ulong instanceID);

        public void Play()
        {
            ParticleEmitterComponent_Play(gameObject.scene, gameObject.GetInstanceID());
        }

        public void Pause()
        {
            ParticleEmitterComponent_Stop(gameObject.scene, gameObject.GetInstanceID());
        }

        public void Stop()
        {
            ParticleEmitterComponent_ResetSystem(gameObject.scene, gameObject.GetInstanceID());
            ParticleEmitterComponent_Stop(gameObject.scene, gameObject.GetInstanceID());
        }


        [DllImport("__Internal")] private static extern bool ParticleEmitterComponent_GetIsPlaying(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void ParticleEmitterComponent_SetIsPlaying(uint sceneID, ulong instanceID, bool value);

        public bool isPlaying
        {
            get { return ParticleEmitterComponent_GetIsPlaying(gameObject.scene, gameObject.GetInstanceID()); }
            set { ParticleEmitterComponent_SetIsPlaying(gameObject.scene, gameObject.GetInstanceID(), value); }
        }
    }
}