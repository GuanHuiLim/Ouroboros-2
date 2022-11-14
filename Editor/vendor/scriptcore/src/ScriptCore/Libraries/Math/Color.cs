namespace Ouroboros
{
    public struct Color
    {
        public float r;
        public float g;
        public float b;
        public float a;

        public Color(float r, float g, float b)
        {
            this.r = r;
            this.g = g;
            this.b = b;
            this.a = 1.0f;
        }
        public Color(float r, float g, float b, float a)
        {
            this.r = r;
            this.g = g;
            this.b = b;
            this.a = a;
        }
        public Color (int r, int g, int b, int a)
        {
            this.r = (float)r / 255.0f;
            this.g = (float)g / 255.0f;
            this.b = (float)b / 255.0f;
            this.a = (float)a / 255.0f;
        }

        public float this[int index]
        {
            get
            {
                switch(index)
                {
                    case 0: return r;
                    case 1: return g;
                    case 2: return b;
                    case 3: return a;
                    default: return 0;
                }
            }
            set
            {
                switch (index)
                {
                    case 0: r = value; break;
                    case 1: g = value; break;
                    case 2: b = value; break;
                    case 3: a = value; break;
                }
            }
        }
        public static Color yellow { get { return new Color(1.0f, 0.92f, 0.016f); } }
        public static Color clear { get { return new Color(0.0f, 0.0f, 0.0f, 0.0f); } }
        public static Color grey { get { return new Color(0.5f, 0.5f, 0.5f); } }
        public static Color gray { get { return new Color(0.5f, 0.5f, 0.5f); } }
        public static Color magenta { get { return new Color(1.0f, 0.0f, 1.0f); } }
        public static Color cyan { get { return new Color(0.0f, 1.0f, 1.0f); } }
        public static Color red { get { return new Color(1.0f, 0.0f, 0.0f); } }
        public static Color black { get { return new Color(0.0f, 0.0f, 0.0f); } }
        public static Color white { get { return new Color(1.0f, 1.0f, 1.0f); } }
        public static Color blue { get { return new Color(0.0f, 0.0f, 1.0f); } }
        public static Color green { get { return new Color(0.0f, 1.0f, 0.0f); } }

        // public static Color HSVToRGB(float H, float S, float V);
        // public static void RGBToHSV(Color rgbColor, out float H, out float S, out float V);
        public static Color Lerp(Color a, Color b, float t)
        {
            return new Color
            (
                Mathf.Lerp(a.r, b.r, t),
                Mathf.Lerp(a.g, b.g, t),
                Mathf.Lerp(a.b, b.b, t),
                Mathf.Lerp(a.a, b.a, t)
            );
        }
        public static Color LerpUnclamped(Color a, Color b, float t)
        {
            return new Color
            (
                Mathf.LerpUnclamped(a.r, b.r, t),
                Mathf.LerpUnclamped(a.g, b.g, t),
                Mathf.LerpUnclamped(a.b, b.b, t),
                Mathf.LerpUnclamped(a.a, b.a, t)
            );
        }

        public bool Equals(Color other)
        {
            return Mathf.Approximately(r, other.r) && Mathf.Approximately(g, other.g) 
                && Mathf.Approximately(b, other.b) && Mathf.Approximately(a, other.a);
        }
        public override bool Equals(object other)
        {
            return base.Equals(other);
        }
        public override int GetHashCode()
        {
            return base.GetHashCode();
        }
        public override string ToString()
        {
            return "RGBA(" + r + ", " + g + ", " + b + ", " + a + ")";
        }

        public static Color operator +(Color a, Color b)
        {
            return new Color(a.r + b.r, a.g + b.g, a.b + b.b, a.a + b.a);
        }
        public static Color operator -(Color a, Color b)
        {
            return new Color(a.r - b.r, a.g - b.g, a.b - b.b, a.a - b.a);
        }
        public static Color operator *(float b, Color a)
        {
            return new Color(a.r * b, a.g * b, a.b * b, a.a * b);
        }
        public static Color operator *(Color a, float b)
        {
            return new Color(a.r * b, a.g * b, a.b * b, a.a * b);
        }
        public static Color operator *(Color a, Color b)
        {
            return new Color(a.r * b.r, a.g * b.g, a.b * b.b, a.a * b.a);
        }
        public static Color operator /(Color a, float b)
        {
            return new Color(a.r / b, a.g / b, a.b / b, a.a / b);
        }
        public static bool operator ==(Color lhs, Color rhs)
        {
            return Mathf.Approximately(lhs.r, rhs.r) && Mathf.Approximately(lhs.g, rhs.g)
                && Mathf.Approximately(lhs.b, rhs.b) && Mathf.Approximately(lhs.a, rhs.a);
        }
        public static bool operator !=(Color lhs, Color rhs)
        {
            return !(lhs == rhs);
        }

        //public static implicit operator Color(Vector4 v);
        //public static implicit operator Vector4(Color c);
    }
}