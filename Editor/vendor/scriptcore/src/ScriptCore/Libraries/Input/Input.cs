using System.Runtime.InteropServices;

namespace Ouroboros
{
    public static class Input
    {
        [DllImport("__Internal")]
        public static extern float GetAxis(string axisName);

        /*-----------------------------------------------------------------------------*/
        /* Keyboard Input Functions                                                    */
        /*-----------------------------------------------------------------------------*/

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
        public static extern KeyCode GetKeyPressed();

        [DllImport("__Internal")]
        public static extern KeyCode GetKeyHeld();

        [DllImport("__Internal")]
        public static extern KeyCode GetKeyReleased();

        /*-----------------------------------------------------------------------------*/
        /* Mouse Input Functions                                                       */
        /*-----------------------------------------------------------------------------*/

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
        public static extern MouseCode GetMouseButtonPressed();

        [DllImport("__Internal")]
        public static extern MouseCode GetMouseButtonHeld();

        [DllImport("__Internal")]
        public static extern MouseCode GetMouseButtonReleased();

        /*-----------------------------------------------------------------------------*/
        /* Controller Input Functions                                                  */
        /*-----------------------------------------------------------------------------*/

        [DllImport("__Internal")]
        public static extern bool IsAnyControllerButtonPressed();

        [DllImport("__Internal")]
        public static extern bool IsAnyControllerButtonHeld();

        [DllImport("__Internal")]
        public static extern bool IsAnyControllerButtonReleased();

        [DllImport("__Internal")]
        public static extern bool IsControllerButtonPressed(ControllerButtonCode button);

        [DllImport("__Internal")]
        public static extern bool IsControllerButtonHeld(ControllerButtonCode button);

        [DllImport("__Internal")]
        public static extern bool IsControllerButtonReleased(ControllerButtonCode button);

        [DllImport("__Internal")]
        public static extern ControllerButtonCode GetControllerButtonPressed();

        [DllImport("__Internal")]
        public static extern ControllerButtonCode GetControllerButtonHeld();

        [DllImport("__Internal")]
        public static extern ControllerButtonCode GetControllerButtonReleased();

        [DllImport("__Internal")]
        public static extern bool IsAnyControllerAxis();

        [DllImport("__Internal")]
        public static extern float GetControllerAxisValue(ControllerAxisCode axis);

        [DllImport("__Internal")]
        public static extern ControllerAxisCode GetControllerAxis();

        [DllImport("__Internal")]
        public static extern bool SetControllerVibration(float time, float intensity);

        [DllImport("__Internal")]
        private static extern bool SetControllerVibration_HighLow(float time, float low_frequency_intensity, float high_frequency_intensity);

        public static bool SetControllerVibration(float time, float low_frequency_intensity, float high_frequency_intensity)
        {
            return SetControllerVibration_HighLow(time, low_frequency_intensity, high_frequency_intensity);
        }

        [DllImport("__Internal")]
        public static extern void StopControllerVibration();
    }
}