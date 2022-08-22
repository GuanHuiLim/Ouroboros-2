using System.Runtime.InteropServices;

namespace Ouroboros
{
    public class Debug
    {
        [DllImport("__Internal")]
        private static extern void Log(string msg);

        /*********************************************************************************//*!
        \brief      Logs a message to the engine's logger

        \param      msg
                string or object to be converted to string representation for display.
        *//**********************************************************************************/
        public static void Log(object msg)
        {
            Log((msg == null) ? "null" : msg.ToString());
        }

        [DllImport("__Internal")]
        private static extern void LogInfo(string msg);

        /*********************************************************************************//*!
        \brief      A variant of Debug.Log that logs an info message to the console.

        \param      msg
                string or object to be converted to string representation for display.
        *//**********************************************************************************/
        public static void LogInfo(object msg)
        {
            LogInfo((msg == null) ? "null" : msg.ToString());
        }

        [DllImport("__Internal")]
        private static extern void LogWarning(string msg);

        /*********************************************************************************//*!
        \brief      A variant of Debug.Log that logs an info message to the console.

        \param      msg
                string or object to be converted to string representation for display.
        *//**********************************************************************************/
        public static void LogWarning(object msg)
        {
            LogWarning((msg == null) ? "null" : msg.ToString());
        }

        [DllImport("__Internal")]
        private static extern void LogError(string msg);

        /*********************************************************************************//*!
        \brief      A variant of Debug.Log that logs an error message to the console.

        \param      msg
                string or object to be converted to string representation for display.
        *//**********************************************************************************/
        public static void LogError(object msg)
        {
            LogError((msg == null) ? "null" : msg.ToString());
        }

        [DllImport("__Internal")]
        private static extern void LogCritical(string msg);

        /*********************************************************************************//*!
        \brief      A variant of Debug.Log that logs a critical error message to the console.

        \param      msg
                string or object to be converted to string representation for display.
        *//**********************************************************************************/
        public static void LogCritical(object msg)
        {
            LogCritical((msg == null) ? "null" : msg.ToString());
        }
    }
}
