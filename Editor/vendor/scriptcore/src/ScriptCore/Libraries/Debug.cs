using System.Runtime.InteropServices;
using System.Diagnostics;

namespace Ouroboros
{
    public static class Debug
    {
        [DllImport("__Internal")] private static extern void Log(string filename, int lineNumber, string msg);

        /*********************************************************************************//*!
        \brief      Logs a message to the engine's logger

        \param      msg
                string or object to be converted to string representation for display.
        *//**********************************************************************************/
        public static void Log(object msg)
        {
            StackFrame frame = new StackFrame(1, true);
            Log(frame.GetFileName(), frame.GetFileLineNumber(), (msg == null) ? "null" : msg.ToString());
        }

        [DllImport("__Internal")] private static extern void LogInfo(string filename, int lineNumber, string msg);

        /*********************************************************************************//*!
        \brief      A variant of Debug.Log that logs an info message to the console.

        \param      msg
                string or object to be converted to string representation for display.
        *//**********************************************************************************/
        public static void LogInfo(object msg)
        {
            StackFrame frame = new StackFrame(1, true);
            LogInfo(frame.GetFileName(), frame.GetFileLineNumber(), (msg == null) ? "null" : msg.ToString());
        }

        [DllImport("__Internal")] private static extern void LogWarning(string filename, int lineNumber, string msg);

        /*********************************************************************************//*!
        \brief      A variant of Debug.Log that logs an info message to the console.

        \param      msg
                string or object to be converted to string representation for display.
        *//**********************************************************************************/
        public static void LogWarning(object msg)
        {
            StackFrame frame = new StackFrame(1, true);
            LogWarning(frame.GetFileName(), frame.GetFileLineNumber(), (msg == null) ? "null" : msg.ToString());
        }

        [DllImport("__Internal")] private static extern void LogError(string filename, int lineNumber, string msg);

        /*********************************************************************************//*!
        \brief      A variant of Debug.Log that logs an error message to the console.

        \param      msg
                string or object to be converted to string representation for display.
        *//**********************************************************************************/
        public static void LogError(object msg)
        {
            StackFrame frame = new StackFrame(1, true);
            LogError(frame.GetFileName(), frame.GetFileLineNumber(), (msg == null) ? "null" : msg.ToString());
        }

        [DllImport("__Internal")] private static extern void LogCritical(string filename, int lineNumber, string msg);

        /*********************************************************************************//*!
        \brief      A variant of Debug.Log that logs a critical error message to the console.

        \param      msg
                string or object to be converted to string representation for display.
        *//**********************************************************************************/
        public static void LogCritical(object msg)
        {
            StackFrame frame = new StackFrame(1, true);
            LogCritical(frame.GetFileName(), frame.GetFileLineNumber(), (msg == null) ? "null" : msg.ToString());
        }

        [DllImport("__Internal")] private static extern void Debug_DrawLine(Vector3 p0, Vector3 p1);
        [DllImport("__Internal")] private static extern void Debug_DrawLine_Color(Vector3 p0, Vector3 p1, Color color);

        public static void DrawLine(Vector3 p0, Vector3 p1)
        {
            Debug_DrawLine(p0, p1);
        }

        public static void DrawLine(Vector3 p0, Vector3 p1, Color color)
        {
            Debug_DrawLine_Color(p0, p1, color);
        }

        [DllImport("__Internal")] private static extern void Debug_DrawWireCube(Vector3 center, Vector3 size);
        [DllImport("__Internal")] private static extern void Debug_DrawWireCube_Color(Vector3 center, Vector3 size, Color color);

        public static void DrawWireCube(Vector3 center, Vector3 size)
        {
            Debug_DrawWireCube(center, size);
        }

        public static void DrawWireCube(Vector3 center, Vector3 size, Color color)
        {
            Debug_DrawWireCube_Color(center, size, color);
        }

        [DllImport("__Internal")] private static extern void Debug_DrawWireSphere(Vector3 center, float radius);
        [DllImport("__Internal")] private static extern void Debug_DrawWireSphere_Color(Vector3 center, float radius, Color color);

        public static void DrawWireSphere(Vector3 center, float radius)
        {
            Debug_DrawWireSphere(center, radius);
        }

        public static void DrawWireSphere(Vector3 center, float radius, Color color)
        {
            Debug_DrawWireSphere_Color(center, radius, color);
        }
    }
}
