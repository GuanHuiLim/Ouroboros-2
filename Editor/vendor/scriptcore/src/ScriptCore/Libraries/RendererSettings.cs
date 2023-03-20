using System.Runtime.InteropServices;

namespace Ouroboros
{
    public static class RendererSettings
    {
        public static class SSAO
        {
            [DllImport("__Internal")] private static extern float RendererSettings_SSAO_GetRadius();
            [DllImport("__Internal")] private static extern void RendererSettings_SSAO_SetRadius(float value);
            public static float Radius
            {
                get { return RendererSettings_SSAO_GetRadius(); }
                set { RendererSettings_SSAO_SetRadius(value); }
            }

            [DllImport("__Internal")] private static extern float RendererSettings_SSAO_GetBias();
            [DllImport("__Internal")] private static extern void RendererSettings_SSAO_SetBias(float value);
            public static float Bias
            {
                get { return RendererSettings_SSAO_GetBias(); }
                set { RendererSettings_SSAO_SetBias(value); }
            }

            [DllImport("__Internal")] private static extern float RendererSettings_SSAO_Getintensity();
            [DllImport("__Internal")] private static extern void RendererSettings_SSAO_Setintensity(float value);
            public static float Intensity
            {
                get { return RendererSettings_SSAO_Getintensity(); }
                set { RendererSettings_SSAO_Setintensity(value); }
            }
        }
        public static class Lighting
        {
            [DllImport("__Internal")] private static extern float RendererSettings_Lighting_GetAmbient();
            [DllImport("__Internal")] private static extern void RendererSettings_Lighting_SetAmbient(float value);
            public static float Ambient
            {
                get { return RendererSettings_Lighting_GetAmbient(); }
                set { RendererSettings_Lighting_SetAmbient(value); }
            }

            [DllImport("__Internal")] private static extern float RendererSettings_Lighting_GetMaxBias();
            [DllImport("__Internal")] private static extern void RendererSettings_Lighting_SetMaxBias(float value);
            public static float MaxBias
            {
                get { return RendererSettings_Lighting_GetMaxBias(); }
                set { RendererSettings_Lighting_SetMaxBias(value); }
            }

            [DllImport("__Internal")] private static extern float RendererSettings_Lighting_GetBiasMultiplier();
            [DllImport("__Internal")] private static extern void RendererSettings_Lighting_SetBiasMultiplier(float value);
            public static float BiasMultiplier
            {
                get { return RendererSettings_Lighting_GetBiasMultiplier(); }
                set { RendererSettings_Lighting_SetBiasMultiplier(value); }
            }
        }
        public static class Bloom
        {
            [DllImport("__Internal")] private static extern float RendererSettings_Bloom_GetThreshold();
            [DllImport("__Internal")] private static extern void RendererSettings_Bloom_SetThreshold(float value);
            public static float Threshold
            {
                get { return RendererSettings_Bloom_GetThreshold(); }
                set { RendererSettings_Bloom_SetThreshold(value); }
            }

            [DllImport("__Internal")] private static extern float RendererSettings_Bloom_GetSoftThreshold();
            [DllImport("__Internal")] private static extern void RendererSettings_Bloom_SetSoftThreshold(float value);
            public static float SoftThreshold
            {
                get { return RendererSettings_Bloom_GetSoftThreshold(); }
                set { RendererSettings_Bloom_SetSoftThreshold(value); }
            }
        }
        public static class ColorCorrection
        {
            [DllImport("__Internal")] private static extern float RendererSettings_ColourCorrection_GetHighlightThreshold();
            [DllImport("__Internal")] private static extern void RendererSettings_ColourCorrection_SetHighlightThreshold(float value);

            public static float HighlightThreshold
            {
                get { return RendererSettings_ColourCorrection_GetHighlightThreshold(); }
                set { RendererSettings_ColourCorrection_SetHighlightThreshold(value); }
            }

            [DllImport("__Internal")] private static extern float RendererSettings_ColourCorrection_GetShadowThreshold();
            [DllImport("__Internal")] private static extern void RendererSettings_ColourCorrection_SetShadowThreshold(float value);
            public static float ShadowThreshold
            {
                get { return RendererSettings_ColourCorrection_GetShadowThreshold(); }
                set { RendererSettings_ColourCorrection_SetShadowThreshold(value); }
            }

            [DllImport("__Internal")] private static extern Color RendererSettings_ColourCorrection_GetShadowColour();
            [DllImport("__Internal")] private static extern void RendererSettings_ColourCorrection_SetShadowColour(Color value);
            public static Color ShadowColor
            {
                get { return RendererSettings_ColourCorrection_GetShadowColour(); }
                set { RendererSettings_ColourCorrection_SetShadowColour(value); }
            }

            [DllImport("__Internal")] private static extern Color RendererSettings_ColourCorrection_GetMidtonesColour();
            [DllImport("__Internal")] private static extern void RendererSettings_ColourCorrection_SetMidtonesColour(Color value);
            public static Color MidtonesColor
            {
                get { return RendererSettings_ColourCorrection_GetMidtonesColour(); }
                set { RendererSettings_ColourCorrection_SetMidtonesColour(value); }
            }

            [DllImport("__Internal")] private static extern Color RendererSettings_ColourCorrection_GetHighlightColour();
            [DllImport("__Internal")] private static extern void RendererSettings_ColourCorrection_SetHighlightColour(Color value);
            public static Color HighlightColor
            {
                get { return RendererSettings_ColourCorrection_GetHighlightColour(); }
                set { RendererSettings_ColourCorrection_SetHighlightColour(value); }
            }
        }
        public static class Vignette
        {
            [DllImport("__Internal")] private static extern Color RendererSettings_Vignette_GetColour();
            [DllImport("__Internal")] private static extern void RendererSettings_Vignette_SetColour(Color value);
            public static Color Color
            {
                get { return RendererSettings_Vignette_GetColour(); }
                set { RendererSettings_Vignette_SetColour(value); }
            }

            [DllImport("__Internal")] private static extern float RendererSettings_Vignette_GetInnerRadius();
            [DllImport("__Internal")] private static extern void RendererSettings_Vignette_SetInnerRadius(float value);
            public static float InnerRadius
            {
                get { return RendererSettings_Vignette_GetInnerRadius(); }
                set { RendererSettings_Vignette_SetInnerRadius(value); }
            }

            [DllImport("__Internal")] private static extern float RendererSettings_Vignette_GetOuterRadius();
            [DllImport("__Internal")] private static extern void RendererSettings_Vignette_SetOuterRadius(float value);
            public static float OuterRadius
            {
                get { return RendererSettings_Vignette_GetOuterRadius(); }
                set { RendererSettings_Vignette_SetOuterRadius(value); }
            }
        }
    }
}