using System.Runtime.InteropServices;

namespace Ouroboros
{
    public static class Time
    {
        [DllImport("__Internal")] private static extern float Time_GetTimeScale();
        [DllImport("__Internal")] private static extern void Time_SetTimeScale(float timeScale);

        public static float timeScale
        {
            get { return Time_GetTimeScale(); }
            set { Time_SetTimeScale(value); }
        }

        [DllImport("__Internal")] private static extern float Time_GetDeltaTime();

        public static float deltaTime
        {
            get { return Time_GetDeltaTime(); }
        }

        [DllImport("__Internal")] private static extern float Time_GetUnscaledDeltaTime();

        public static float unscaledDeltaTime
        {
            get { return Time_GetUnscaledDeltaTime(); }
        }

        //[DllImport("__Internal")] private static extern float Time_GetFixedDeltaTime();

        //public static float fixedDeltaTime
        //{
        //    get { return Time_GetFixedDeltaTime(); }
        //}

        //[DllImport("__Internal")] private static extern float Time_GetFixedUnscaledDeltaTime();

        //public static float fixedUnscaledDeltaTime
        //{
        //    get { return Time_GetFixedUnscaledDeltaTime(); }
        //}
    }
}