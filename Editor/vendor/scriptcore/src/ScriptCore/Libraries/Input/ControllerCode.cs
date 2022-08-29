namespace Ouroboros
{
    public enum ControllerButtonCode
    {
        A = 0,
        B,
        X,
        Y,
        BACK,
        GUIDE,
        START,
        LEFTSTICK,
        RIGHTSTICK,
        LEFTSHOULDER,
        RIGHTSHOULDER,
        DPAD_UP,
        DPAD_DOWN,
        DPAD_LEFT,
        DPAD_RIGHT,
        MISC1,    /* Xbox Series X share button, PS5 microphone button, Nintendo Switch Pro capture button */
        PADDLE1,  /* Xbox Elite paddle P1 */
        PADDLE2,  /* Xbox Elite paddle P3 */
        PADDLE3,  /* Xbox Elite paddle P2 */
        PADDLE4,  /* Xbox Elite paddle P4 */
        TOUCHPAD, /* PS4/PS5 touchpad button */
    }

    public enum ControllerAxisCode
    {
        LEFTX = 0,
        LEFTY,
        RIGHTX,
        RIGHTY,
        TRIGGERLEFT,
        TRIGGERRIGHT,
    }
}

