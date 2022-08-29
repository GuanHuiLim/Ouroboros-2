using System.Runtime.InteropServices;

namespace Ouroboros
{
    public static class Application
    {
        [DllImport("__Internal")] private static extern void Application_Quit();

        public static void Quit()
        {
            Application_Quit();
        }
    }
}