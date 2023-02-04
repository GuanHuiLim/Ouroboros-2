using System;
using System.Runtime.InteropServices;

namespace Ouroboros
{
    public class Light : Component
    {
        [DllImport("__Internal")] private static extern Color LightComponent_GetColor(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void LightComponent_SetColor(uint sceneID, ulong instanceID, Color value);

        public Color color
        {
            get { return LightComponent_GetColor(gameObject.scene, gameObject.GetInstanceID()); }
            set { LightComponent_SetColor(gameObject.scene, gameObject.GetInstanceID(), value); }
        }

        [DllImport("__Internal")] private static extern float LightComponent_GetIntensity(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void LightComponent_SetIntensity(uint sceneID, ulong instanceID, float value);

        public float intensity
        {
            get { return LightComponent_GetIntensity(gameObject.scene, gameObject.GetInstanceID()); }
            set { LightComponent_SetIntensity(gameObject.scene, gameObject.GetInstanceID(), value); }
        }

        [DllImport("__Internal")] private static extern float LightComponent_GetRadius(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void LightComponent_SetRadius(uint sceneID, ulong instanceID, float value);

        public float radius
        {
            get { return LightComponent_GetRadius(gameObject.scene, gameObject.GetInstanceID()); }
            set { LightComponent_SetRadius(gameObject.scene, gameObject.GetInstanceID(), value); }
        }

        [DllImport("__Internal")] private static extern bool LightComponent_GetProduceShadows(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void LightComponent_SetProduceShadows(uint sceneID, ulong instanceID, bool value);

        public bool produceShadows
        {
            get { return LightComponent_GetProduceShadows(gameObject.scene, gameObject.GetInstanceID()); }
            set { LightComponent_SetProduceShadows(gameObject.scene, gameObject.GetInstanceID(), value); }
        }
    }
}