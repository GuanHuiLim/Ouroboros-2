namespace Ouroboros
{
    public static class Mathf
    {
        public const float PI = 3.14159274F;
        public const float Infinity = float.PositiveInfinity;
        public const float NegativeInfinity = float.NegativeInfinity;
        public const float Deg2Rad = 0.0174532924F; // PI / 180.0f;
        public const float Rad2Deg = 57.29578F; // 180.0f / PI;
        public static readonly float Epsilon = float.Epsilon;

        public static float Abs(float value)
        {
            return (value < 0) ? -value : value;
        }

        public static int Abs(int value)
        {
            return (value < 0) ? -value : value;
        }

        public static float Acos(float f)
        {
            return (float)System.Math.Acos(f);
        }

        public static float Asin(float f)
        {
            return (float)System.Math.Asin(f);
        }

        public static float Atan(float f)
        {
            return (float)System.Math.Atan(f);
        }

        public static float Atan2(float y, float x)
        {
            return (float)System.Math.Atan2(y, x);
        }

        public static bool Approximately(float a, float b)
        {
            return Abs(a - b) <= Epsilon;
        }

        public static float Ceil(float f)
        {
            return (float)System.Math.Ceiling(f);
        }

        public static int CeilToInt(float f)
        {
            return (int)System.Math.Ceiling(f);
        }

        public static int Clamp(int value, int min, int max)
        {
            if (value < min)
                value = min;
            if (value > max)
                value = max;
            return value;
        }

        public static float Clamp(float value, float min, float max)
        {
            if (value < min)
                value = min;
            if (value > max)
                value = max;
            return value;
        }

        public static float Clamp01(float value)
        {
            if (value < 0)
                value = 0;
            if (value > 1)
                value = 1;
            return value;
        }

        // public static int ClosestPowerOfTwo(int value)

        //public static Color CorrelatedColorTemperatureToRGB(float kelvin)

        public static float Cos(float f)
        {
            return (float)System.Math.Cos(f);
        }

        public static float DeltaAngle(float current, float target)
        {
            float f = target - current;
            while (f < -180.0f) f += 360.0f;
            while (f >= 180.0f) f -= 360.0f;
            return f;
        }

        public static float Exp(float power)
        {
            return (float)System.Math.Exp(power);
        }

        // public static ushort FloatToHalf(float val)

        public static float Floor(float f)
        {
            return (float)System.Math.Floor(f);
        }

        public static int FloorToInt(float f)
        {
            return (int)System.Math.Floor(f);
        }

        // public static float Gamma(float value, float absmax, float gamma)

        // public static float GammaToLinearSpace(float value)

        // public static float HalfToFloat(ushort val)

        public static float InverseLerp(float a, float b, float value)
        {
            float t = 0;
            if (value < a) t = 0;
            else if (value > b) t = 1;
            else t = (value - a) / (b - a);
            return t;
        }

        // public static bool IsPowerOfTwo(int value)

        public static float Lerp(float a, float b, float t)
        {
            if (t < 0) t = 0;
            if (t > 1) t = 1;
            return ((b - a) * t) + a;
        }

        //public static float LerpAngle(float a, float b, float t)
        //{
        //    if (t < 0) t = 0;
        //    if (t > 1) t = 1;
        //    float f = ((b - a) * t) + a;

        //    return f;
        //}

        public static float LerpUnclamped(float a, float b, float t)
        {
            return ((b - a) * t) + a;
        }

        // public static float LinearToGammaSpace(float value)

        public static float Log(float f, float p)
        {
            return (float)System.Math.Log(f, p);
        }

        public static float Log(float f)
        {
            return (float)System.Math.Log(f);
        }

        public static float Log10(float f)
        {
            return (float)System.Math.Log10(f);
        }

        public static int Max(int a, int b)
        {
            return (a > b) ? a : b;
        }

        public static float Max(params float[] values)
        {
            float max = values[0];
            for(int i = 1; i < values.Length; ++i)
            {
                if (values[i] > max)
                    max = values[i];
            }
            return max;
        }

        public static float Max(float a, float b)
        {
            return (a > b) ? a : b;
        }

        public static int Max(params int[] values)
        {
            int max = values[0];
            for (int i = 1; i < values.Length; ++i)
            {
                if (values[i] > max)
                    max = values[i];
            }
            return max;
        }

        public static int Min(params int[] values)
        {
            int min = values[0];
            for (int i = 1; i < values.Length; ++i)
            {
                if (values[i] < min)
                    min = values[i];
            }
            return min;
        }

        public static int Min(int a, int b)
        {
            return (a < b) ? a : b;
        }

        public static float Min(params float[] values)
        {
            float min = values[0];
            for (int i = 1; i < values.Length; ++i)
            {
                if (values[i] < min)
                    min = values[i];
            }
            return min;
        }

        public static float Min(float a, float b)
        {
            return (a < b) ? a : b;
        }

        public static float MoveTowards(float current, float target, float maxDelta)
        {
            float result = target;
            if (Abs(target - current) > maxDelta)
                result = current + (Sign(target - current) * maxDelta);
            return result;
        }

        //public static float MoveTowardsAngle(float current, float target, float maxDelta)
        //{

        //}

        // public static int NextPowerOfTwo(int value)

        // public static float PerlinNoise(float x, float y)

        // public static float PingPong(float t, float length)

        public static float Pow(float f, float p)
        {
            return (float)System.Math.Pow(f, p);
        }

        public static float Repeat(float t, float length)
        {
            t -= (int)(t / length) * length;
            return t;
        }

        public static float Round(float f)
        {
            return (float)System.Math.Round(f);
        }

        public static int RoundToInt(float f)
        {
            return (int)System.Math.Round(f);
        }

        public static float Sign(float f)
        {
            return (f >= 0) ? 1 : -1;
        }

        public static float Sin(float f)
        {
            return (float)System.Math.Sin(f);
        }

        // public static float SmoothDamp(float current, float target, ref float currentVelocity, float smoothTime, float maxSpeed)

        // public static float SmoothDamp(float current, float target, ref float currentVelocity, float smoothTime)

        //public static float SmoothDamp(float current, float target, ref float currentVelocity, float smoothTime, [DefaultValue("Mathf.Infinity")] float maxSpeed, [DefaultValue("Time.deltaTime")] float deltaTime);

        // public static float SmoothDampAngle(float current, float target, ref float currentVelocity, float smoothTime, float maxSpeed)

        // public static float SmoothDampAngle(float current, float target, ref float currentVelocity, float smoothTime)

        //public static float SmoothDampAngle(float current, float target, ref float currentVelocity, float smoothTime, [DefaultValue("Mathf.Infinity")] float maxSpeed, [DefaultValue("Time.deltaTime")] float deltaTime);

        //public static float SmoothStep(float from, float to, float t)
        //{
        //    // t = t < 0.5 ? 2 * t * t : 1 - Pow(-2 * t + 2, 2) / 2;
        //    t = -(Cos(PI * t) - 1) / 2;
        //    return Lerp(from, to, t);
        //}

        public static float Sqrt(float f)
        {
            return (float)System.Math.Sqrt(f);
        }

        public static float Tan(float f)
        {
            return (float)System.Math.Tan(f);
        }
    }
}