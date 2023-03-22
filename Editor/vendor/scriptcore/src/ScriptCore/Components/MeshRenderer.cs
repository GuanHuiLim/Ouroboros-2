using System;
using System.Runtime.InteropServices;

namespace Ouroboros
{
    public class MeshRenderer : Component
    {
        [DllImport("__Internal")] private static extern Color MeshRendererComponent_GetEmissiveColor(UInt32 sceneID, UInt64 instanceID);
        [DllImport("__Internal")] private static extern void MeshRendererComponent_SetEmissiveColor(UInt32 sceneID, UInt64 instanceID, Color value);

        public Color emissiveColor
        {
            get { return MeshRendererComponent_GetEmissiveColor(gameObject.scene, gameObject.GetInstanceID()); }
            set { MeshRendererComponent_SetEmissiveColor(gameObject.scene, gameObject.GetInstanceID(), value); }
        }
    }
}