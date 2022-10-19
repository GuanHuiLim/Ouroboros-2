namespace Ouroboros
{
    public struct Colour
    {
        public float r;
        public float g;
        public float b;
        public float a;

        public Colour(float r, float g, float b)
        {
            this.r = r;
            this.g = g;
            this.b = b;
            this.a = 1.0f;
        }
        public Colour(float r, float g, float b, float a)
        {
            this.r = r;
            this.g = g;
            this.b = b;
            this.a = a;
        }
        public Colour (int r, int g, int b, int a)
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
        public static Colour yellow { get { return new Colour(1.0f, 0.92f, 0.016f); } }
        public static Colour clear { get { return new Colour(0.0f, 0.0f, 0.0f, 0.0f); } }
        public static Colour grey { get { return new Colour(0.5f, 0.5f, 0.5f); } }
        public static Colour gray { get { return new Colour(0.5f, 0.5f, 0.5f); } }
        public static Colour magenta { get { return new Colour(1.0f, 0.0f, 1.0f); } }
        public static Colour cyan { get { return new Colour(0.0f, 1.0f, 1.0f); } }
        public static Colour red { get { return new Colour(1.0f, 0.0f, 0.0f); } }
        public static Colour black { get { return new Colour(0.0f, 0.0f, 0.0f); } }
        public static Colour white { get { return new Colour(1.0f, 1.0f, 1.0f); } }
        public static Colour blue { get { return new Colour(0.0f, 0.0f, 1.0f); } }
        public static Colour green { get { return new Colour(0.0f, 1.0f, 0.0f); } }

        // public static Colour HSVToRGB(float H, float S, float V);
        // public static void RGBToHSV(Colour rgbColour, out float H, out float S, out float V);
        public static Colour Lerp(Colour a, Colour b, float t)
        {
            return new Colour
            (
                Mathf.Lerp(a.r, b.r, t),
                Mathf.Lerp(a.g, b.g, t),
                Mathf.Lerp(a.b, b.b, t),
                Mathf.Lerp(a.a, b.a, t)
            );
        }
        public static Colour LerpUnclamped(Colour a, Colour b, float t)
        {
            return new Colour
            (
                Mathf.LerpUnclamped(a.r, b.r, t),
                Mathf.LerpUnclamped(a.g, b.g, t),
                Mathf.LerpUnclamped(a.b, b.b, t),
                Mathf.LerpUnclamped(a.a, b.a, t)
            );
        }

        public bool Equals(Colour other)
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

        public static Colour operator +(Colour a, Colour b)
        {
            return new Colour(a.r + b.r, a.g + b.g, a.b + b.b, a.a + b.a);
        }
        public static Colour operator -(Colour a, Colour b)
        {
            return new Colour(a.r - b.r, a.g - b.g, a.b - b.b, a.a - b.a);
        }
        public static Colour operator *(float b, Colour a)
        {
            return new Colour(a.r * b, a.g * b, a.b * b, a.a * b);
        }
        public static Colour operator *(Colour a, float b)
        {
            return new Colour(a.r * b, a.g * b, a.b * b, a.a * b);
        }
        public static Colour operator *(Colour a, Colour b)
        {
            return new Colour(a.r * b.r, a.g * b.g, a.b * b.b, a.a * b.a);
        }
        public static Colour operator /(Colour a, float b)
        {
            return new Colour(a.r / b, a.g / b, a.b / b, a.a / b);
        }
        public static bool operator ==(Colour lhs, Colour rhs)
        {
            return Mathf.Approximately(lhs.r, rhs.r) && Mathf.Approximately(lhs.g, rhs.g)
                && Mathf.Approximately(lhs.b, rhs.b) && Mathf.Approximately(lhs.a, rhs.a);
        }
        public static bool operator !=(Colour lhs, Colour rhs)
        {
            return !(lhs == rhs);
        }

        //public static implicit operator Colour(Vector4 v);
        //public static implicit operator Vector4(Colour c);
    }
}