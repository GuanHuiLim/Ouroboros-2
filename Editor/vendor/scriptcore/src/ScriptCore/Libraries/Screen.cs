using System.Runtime.InteropServices;

namespace Ouroboros
{
    public static class Screen
    {
        [DllImport("__Internal")] private static extern uint Screen_GetWidth();
        [DllImport("__Internal")] private static extern uint Screen_GetHeight();

        public static float width
        {
            get { return Screen_GetWidth(); }
        }

        public static float height
        {
            get { return Screen_GetHeight(); }
        }

        [DllImport("__Internal")] private static extern bool Screen_GetFullScreen();
        [DllImport("__Internal")] private static extern void Screen_SetFullScreen(bool fullscreen);

        public static bool fullScreen
        {
            get { return Screen_GetFullScreen(); }
            set { Screen_SetFullScreen(value); }
        }
    }
}