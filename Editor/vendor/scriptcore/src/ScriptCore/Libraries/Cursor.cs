using System.Runtime.InteropServices;

namespace Ouroboros
{
    public static class Cursor
    {
        [DllImport("__Internal")] private static extern void Cursor_SetVisible(bool isVisible);

        public static bool visible
        {
            set { Cursor_SetVisible(value); }
        }
    }
}