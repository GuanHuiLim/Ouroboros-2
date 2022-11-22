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

        [DllImport("__Internal")] private static extern bool Cursor_GetLocked();
        [DllImport("__Internal")] private static extern void Cursor_SetLocked(bool isVisible);

        public static bool locked
        {
            get { return Cursor_GetLocked(); }
            set { Cursor_SetLocked(value); }
        }

        [DllImport("__Internal")] private static extern void Cursor_SetPosition(int x, int y);

        public static Vector2Int position
        {
            set { Cursor_SetPosition(value.x, value.y); }
        }
    }
}