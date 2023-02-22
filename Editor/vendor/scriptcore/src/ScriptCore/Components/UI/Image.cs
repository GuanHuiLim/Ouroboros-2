using System.Runtime.InteropServices;

namespace Ouroboros
{
    public class Image : Component
    {
        [DllImport("__Internal")] private static extern ulong UIImageComponent_GetSprite(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void UIImageComponent_SetSprite(uint sceneID, ulong instanceID, ulong assetID);

        public Texture sprite
        {
            get
            {
                ulong id = UIImageComponent_GetSprite(gameObject.scene, gameObject.GetInstanceID());
                if (id == 0)
                    return null;
                return new Texture(id);
            }
            set { UIImageComponent_SetSprite(gameObject.scene, gameObject.GetInstanceID(), (value == null) ? 0 : value.GetID()); }
        }

        [DllImport("__Internal")] private static extern Color UIImageComponent_GetColor(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void UIImageComponent_SetColor(uint sceneID, ulong instanceID, Color value);

        public Color color
        {
            get { return UIImageComponent_GetColor(gameObject.scene, gameObject.GetInstanceID()); }
            set { UIImageComponent_SetColor(gameObject.scene, gameObject.GetInstanceID(), value); }
        }
    }
}