using System.Runtime.InteropServices;

namespace Ouroboros
{
    public class Text : Component
    {
        [DllImport("__Internal")] private static extern System.IntPtr UITextComponent_GetText(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void UITextComponent_SetText(uint sceneID, ulong instanceID, string value);

        public string text
        {
            get
            {
                GCHandle textPtr = GCHandle.FromIntPtr(UITextComponent_GetText(gameObject.scene, gameObject.GetInstanceID()));
                string text = (string)textPtr.Target;
                textPtr.Free();
                return text; 
            }
            set { UITextComponent_SetText(gameObject.scene, gameObject.GetInstanceID(), value); }
        }

        [DllImport("__Internal")] private static extern Color UITextComponent_GetColor(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void UITextComponent_SetColor(uint sceneID, ulong instanceID, Color value);

        public Color color
        {
            get { return UITextComponent_GetColor(gameObject.scene, gameObject.GetInstanceID()); }
            set { UITextComponent_SetColor(gameObject.scene, gameObject.GetInstanceID(), value); }
        }

        [DllImport("__Internal")] private static extern float UITextComponent_GetFontSize(uint sceneID, ulong instanceID);
        [DllImport("__Internal")] private static extern void UITextComponent_SetFontSize(uint sceneID, ulong instanceID, float value);

        public float fontSize
        {
            get { return UITextComponent_GetFontSize(gameObject.scene, gameObject.GetInstanceID()); }
            set { UITextComponent_SetFontSize(gameObject.scene, gameObject.GetInstanceID(), value); }
        }
    }
}