using System;
using System.Runtime.InteropServices;

namespace Ouroboros
{
    public class SkinnedMeshRenderer : Component
    {
        [DllImport("__Internal")] private static extern Color SkinMeshRendererComponent_GetEmissiveColor(UInt32 sceneID, UInt64 instanceID);
        [DllImport("__Internal")] private static extern void SkinMeshRendererComponent_SetEmissiveColor(UInt32 sceneID, UInt64 instanceID, Color value);

        public Color emissiveColor
        {
            get { return SkinMeshRendererComponent_GetEmissiveColor(gameObject.scene, gameObject.GetInstanceID()); }
            set { SkinMeshRendererComponent_SetEmissiveColor(gameObject.scene, gameObject.GetInstanceID(), value); }
        }
    }
}