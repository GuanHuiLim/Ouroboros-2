using System.Runtime.InteropServices;

namespace Ouroboros
{
    public static class Input
    {
        [DllImport("__Internal")]
        public static extern float GetAxis(string axisName);

        [DllImport("__Internal")]
        public static extern bool IsAnyKeyPressed();

        [DllImport("__Internal")]
        public static extern bool IsAnyKeyHeld();

        [DllImport("__Internal")]
        public static extern bool IsAnyKeyReleased();

        [DllImport("__Internal")]
        public static extern bool IsKeyPressed(KeyCode key);

        [DllImport("__Internal")]
        public static extern bool IsKeyHeld(KeyCode key);

        [DllImport("__Internal")]
        public static extern bool IsKeyReleased(KeyCode key);

        [DllImport("__Internal")]
        private static extern void GetMousePosition(out int x, out int y);

        public static Vector2 mousePosition
        {
            get
            {
                int x, y;
                GetMousePosition(out x, out y);
                return new Vector2(x, y);
            }
        }

        [DllImport("__Internal")]
        private static extern void GetMouseDelta(out int x, out int y);

        public static Vector2 mouseDelta
        {
            get
            {
                int x, y;
                GetMouseDelta(out x, out y);
                return new Vector2(x, y);
            }
        }

        [DllImport("__Internal")]
        public static extern bool IsAnyMouseButtonPressed();

        [DllImport("__Internal")]
        public static extern bool IsAnyMouseButtonHeld();

        [DllImport("__Internal")]
        public static extern bool IsAnyMouseButtonReleased();

        [DllImport("__Internal")]
        public static extern bool IsMouseButtonPressed(MouseCode button);

        [DllImport("__Internal")]
        public static extern bool IsMouseButtonHeld(MouseCode button);

        [DllImport("__Internal")]
        public static extern bool IsMouseButtonReleased(MouseCode button);


        [DllImport("__Internal")]
        public static extern bool IsControllerButtonPressed(ControllerButtonCode button);

        [DllImport("__Internal")]
        public static extern bool IsControllerButtonHeld(ControllerButtonCode button);

        [DllImport("__Internal")]
        public static extern bool IsControllerButtonReleased(ControllerButtonCode button);

        [DllImport("__Internal")]
        public static extern float GetControllerAxisValue(ControllerAxisCode axis);

    }
}