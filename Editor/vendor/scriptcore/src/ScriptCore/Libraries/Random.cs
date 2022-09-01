namespace Ouroboros
{
    public static class Random
    {
        private static System.Random rnd = new System.Random();

        public static void InitState(int seed)
        {
            rnd = new System.Random(seed);
        }

        public static int Range(int minInclusive, int maxExclusive)
        {
            return rnd.Next(minInclusive, maxExclusive);
        }

        public static float Range(float minInclusive, float maxInclusive)
        {
            return (float)(rnd.NextDouble() * (maxInclusive - minInclusive)) + minInclusive;
        }
    }
}
