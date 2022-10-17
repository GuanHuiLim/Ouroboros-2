namespace Ouroboros
{
    public struct Vector2Int
    {
        public int x;
        public int y;

        public Vector2Int(int x, int y)
        {
            this.x = x;
            this.y = y;
        }

        public int this[int index]
        {
            get
            {
                switch (index)
                {
                    case 0: return x;
                    case 1: return y;
                    default: return 0;
                }
            }
            set
            {
                switch (index)
                {
                    case 0: x = value; break;
                    case 1: y = value; break;
                }
            }
        }
        public static Vector2Int right { get { return new Vector2Int(1, 0); } }
        public static Vector2Int left { get { return new Vector2Int(-1, 0); } }
        public static Vector2Int down { get { return new Vector2Int(0, -1); } }
        public static Vector2Int up { get { return new Vector2Int(0, 1); } }
        public static Vector2Int one { get { return new Vector2Int(1, 1); } }
        public static Vector2Int zero { get { return new Vector2Int(0, 0); } }
        public float magnitude { get { return Mathf.Sqrt((x * x) + (y * y)); } }
        public int sqrMagnitude { get { return (x * x) + (y * y); } }

        public static Vector2Int CeilToInt(Vector2 v)
        {
            return new Vector2Int(Mathf.CeilToInt(v.x), Mathf.CeilToInt(v.y));
        }
        public static float Distance(Vector2Int a, Vector2Int b)
        {
            return (a - b).magnitude;
        }
        public static Vector2Int FloorToInt(Vector2 v)
        {
            return new Vector2Int(Mathf.FloorToInt(v.x), Mathf.FloorToInt(v.y));
        }
        public static Vector2Int Max(Vector2Int lhs, Vector2Int rhs)
        {
            return new Vector2Int(Mathf.Max(lhs.x, rhs.x), Mathf.Max(lhs.y, rhs.y));
        }
        public static Vector2Int Min(Vector2Int lhs, Vector2Int rhs)
        {
            return new Vector2Int(Mathf.Min(lhs.x, rhs.x), Mathf.Min(lhs.y, rhs.y));
        }
        public static Vector2Int RoundToInt(Vector2 v)
        {
            return new Vector2Int(Mathf.RoundToInt(v.x), Mathf.RoundToInt(v.y));
        }
        public static Vector2Int Scale(Vector2Int a, Vector2Int b)
        {
            return new Vector2Int(a.x * b.x, a.y * b.y);
        }
        public void Clamp(Vector2Int min, Vector2Int max)
        {
            x = Mathf.Clamp(x, min.x, max.x);
            y = Mathf.Clamp(y, min.y, max.y);
        }
        public bool Equals(Vector2Int other)
        {
            return x == other.x && y == other.y;
        }
        public override bool Equals(object other)
        {
            return base.Equals(other);
        }
        public override int GetHashCode()
        {
            return base.GetHashCode();
        }
        public void Scale(Vector2Int scale)
        {
            x *= scale.x;
            y *= scale.y;
        }
        public void Set(int x, int y)
        {
            this.x = x;
            this.y = y;
        }
        public override string ToString()
        {
            return "(" + x + ", " + y + ")";
        }

        public static Vector2Int operator +(Vector2Int a, Vector2Int b)
        {
            return new Vector2Int(a.x + b.x, a.y + b.y);
        }
        public static Vector2Int operator -(Vector2Int v)
        {
            return new Vector2Int(-v.x, -v.y);
        }
        public static Vector2Int operator -(Vector2Int a, Vector2Int b)
        {
            return new Vector2Int(a.x - b.x, a.y - b.y);
        }
        public static Vector2Int operator *(Vector2Int a, int b)
        {
            return new Vector2Int(a.x * b, a.y * b);
        }
        public static Vector2Int operator *(int a, Vector2Int b)
        {
            return new Vector2Int(b.x * a, b.y * a);
        }
        public static Vector2Int operator *(Vector2Int a, Vector2Int b)
        {
            return new Vector2Int(a.x * b.x, a.y * b.y);
        }
        public static Vector2Int operator /(Vector2Int a, int b)
        {
            return new Vector2Int(a.x / b, a.y / b);
        }
        public static bool operator ==(Vector2Int lhs, Vector2Int rhs)
        {
            return lhs.x == rhs.x && lhs.y == rhs.y;
        }
        public static bool operator !=(Vector2Int lhs, Vector2Int rhs)
        {
            return !(lhs == rhs);
        }

        public static implicit operator Vector2(Vector2Int v)
        {
            return new Vector2(v.x, v.y);
        }
        public static explicit operator Vector3Int(Vector2Int v)
        {
            return new Vector3Int(v.x, v.y, 0);
        }
    }
}