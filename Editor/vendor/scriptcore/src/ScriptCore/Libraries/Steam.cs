using System.Runtime.InteropServices;

namespace Ouroboros
{
    public static class Steam
    {
        [DllImport("__Internal")] private static extern bool CheckAchievement(string name);

        public static bool CheckAchievementUnlocked(string name)
        {
            return CheckAchievement(name);
        }

        [DllImport("__Internal")] private static extern void UnlockAchievement(string name);

        public static void TryUnlockAchievement(string name)
        {
            UnlockAchievement(name);
        }


        [DllImport("__Internal")] private static extern void SetAchievementStat_Int(string name, int value);
        [DllImport("__Internal")] private static extern int GetAchievementStat_Int(string name);

        public static void TryUnlockAchievement(string name, int newStat)
        {
            SetAchievementStat_Int(name, newStat);
        }

        public static int GetPlayerStatInt(string name)
        {
            return GetAchievementStat_Int(name);
        }

        [DllImport("__Internal")] private static extern void SetAchievementStat_Float(string name, float value);
        [DllImport("__Internal")] private static extern float GetAchievementStat_Float(string name);

        public static void TryUnlockAchievement(string name, float newStat)
        {
            SetAchievementStat_Float(name, newStat);
        }

        public static float GetPlayerStatFloat(string name)
        {
            return GetAchievementStat_Float(name);
        }
    }
}