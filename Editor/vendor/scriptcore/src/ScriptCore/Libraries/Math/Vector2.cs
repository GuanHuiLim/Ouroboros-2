namespace Ouroboros
{
    public struct Vector2
    {
        public float x;
        public float y;

        public Vector2(float x, float y)
        {
            this.x = x;
            this.y = y;
        }

        public float this[int index]
        {
            get
            {
                switch(index)
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

        public static Vector2 right { get { return new Vector2(-1, 0); } }
        public static Vector2 left { get { return new Vector2(1, 0); } }
        public static Vector2 down { get { return new Vector2(0, -1); } }
        public static Vector2 up { get { return new Vector2(0, 1); } }
        public static Vector2 one { get { return new Vector2(1, 1); } }
        public static Vector2 zero { get { return new Vector2(0, 0); } }
        public static Vector2 positiveInfinity { get { return new Vector2(float.PositiveInfinity, float.PositiveInfinity); } }
        public static Vector2 negativeInfinity { get { return new Vector2(float.NegativeInfinity, float.NegativeInfinity); } }

        public float sqrMagnitude { get { return (x * x) + (y * y); } }
        public Vector2 normalized
        {
            get
            {
                Vector2 result = new Vector2();
                float length = magnitude;
                result.x = x / length;
                result.y = y / length;
                return result;
            }
        }
        public float magnitude { get { return Mathf.Sqrt((x * x) + (y * y)); } }

        public static float Angle(Vector2 from, Vector2 to)
        {
            return Mathf.Acos(Dot(from, to) / (from.magnitude * to.magnitude)) * Mathf.Rad2Deg;
        }
        public static Vector2 ClampMagnitude(Vector2 vector, float maxLength)
        {
            float length = vector.magnitude;
            if(length > maxLength)
            {
                vector /= length;
                vector *= maxLength;
            }
            return vector;
        }
        public static float Distance(Vector2 a, Vector2 b)
        {
            Vector2 diff = a - b;
            return diff.magnitude;
        }
        public static float Dot(Vector2 lhs, Vector2 rhs)
        {
            return (lhs.x * rhs.x) + (lhs.y * rhs.y);
        }
        public static Vector2 Lerp(Vector2 a, Vector2 b, float t)
        {
            return new Vector2(Mathf.Lerp(a.x, b.x, t), Mathf.Lerp(a.y, b.y, t));
        }
        public static Vector2 LerpUnclamped(Vector2 a, Vector2 b, float t)
        {
            return new Vector2(Mathf.LerpUnclamped(a.x, b.x, t), Mathf.LerpUnclamped(a.y, b.y, t));
        }
        public static Vector2 Max(Vector2 lhs, Vector2 rhs)
        {
            return new Vector2(Mathf.Max(lhs.x, rhs.x), Mathf.Max(lhs.y, rhs.y));
        }
        public static Vector2 Min(Vector2 lhs, Vector2 rhs)
        {
            return new Vector2(Mathf.Min(lhs.x, rhs.x), Mathf.Min(lhs.y, rhs.y));
        }
        public static Vector2 MoveTowards(Vector2 current, Vector2 target, float maxDistanceDelta)
        {
            Vector2 result = target;
            if (maxDistanceDelta < 0 || (target - current).sqrMagnitude > maxDistanceDelta * maxDistanceDelta)
                result = current + ((target - current).normalized * maxDistanceDelta);
            return result;
        }
        public static Vector2 Perpendicular(Vector2 inDirection)
        {
            return new Vector2(-inDirection.y, inDirection.x);
        }
        public static Vector2 Project(Vector2 inDirection, Vector2 inNormal)
        {
            return (Dot(inDirection, inNormal) / Dot(inNormal, inNormal)) * inNormal;
        }
        public static Vector2 Reflect(Vector2 inDirection, Vector2 inNormal)
        {
            return inDirection - (2.0f * Project(inDirection, inNormal));
        }
        public static Vector2 Scale(Vector2 a, Vector2 b)
        {
            return new Vector2(a.x * b.x, a.y * b.y);
        }
        public static float SignedAngle(Vector2 from, Vector2 to)
        {
            return Mathf.Atan2((from.x * to.y) - (from.y * to.x), (from.x * to.x) + (from.y * to.y)) * Mathf.Rad2Deg;
        }
        // public static Vector2 SmoothDamp(Vector2 current, Vector2 target, ref Vector2 currentVelocity, float smoothTime, float maxSpeed);
        public static float SqrMagnitude(Vector2 a)
        {
            return (a.x * a.x) + (a.y * a.y);
        }
        public bool Equals(Vector2 other)
        {
            return Mathf.Approximately(x, other.x) && Mathf.Approximately(y, other.y);
        }
        public override bool Equals(object other)
        {
            return base.Equals(other);
        }
        public override int GetHashCode()
        {
            return base.GetHashCode();
        }
        public void Normalize()
        {
            float length = magnitude;
            x /= length;
            y /= length;
        }
        public void Scale(Vector2 scale)
        {
            x *= scale.x;
            y *= scale.y;
        }
        public void Set(float newX, float newY)
        {
            x = newX;
            y = newY;
        }
        public float SqrMagnitude()
        {
            return (x * x) + (y * y);
        }
        public override string ToString()
        {
            return "(" + x + ", " + y + ")";
        }

        public static Vector2 operator +(Vector2 a, Vector2 b)
        {
            return new Vector2(a.x + b.x, a.y + b.y);
        }
        public static Vector2 operator -(Vector2 a)
        {
            return new Vector2(-a.x, -a.y);
        }
        public static Vector2 operator -(Vector2 a, Vector2 b)
        {
            return new Vector2(a.x - b.x, a.y - b.y);
        }
        public static Vector2 operator *(float d, Vector2 a)
        {
            return new Vector2(a.x * d, a.y * d);
        }
        public static Vector2 operator *(Vector2 a, float d)
        {
            return new Vector2(a.x * d, a.y * d);
        }
        public static Vector2 operator *(Vector2 a, Vector2 b)
        {
            return new Vector2(a.x * b.x, a.y * b.y);
        }
        public static Vector2 operator /(Vector2 a, float d)
        {
            return new Vector2(a.x / d, a.y / d);
        }
        public static Vector2 operator /(Vector2 a, Vector2 b)
        {
            return new Vector2(a.x / b.x, a.y / b.y);
        }
        public static bool operator ==(Vector2 lhs, Vector2 rhs)
        {
            return Mathf.Approximately(lhs.x, rhs.x) && Mathf.Approximately(lhs.y, rhs.y);
        }
        public static bool operator !=(Vector2 lhs, Vector2 rhs)
        {
            return !(lhs == rhs);
        }

        public static implicit operator Vector2(Vector3 v)
        {
            return new Vector2(v.x, v.y);
        }
        public static implicit operator Vector3(Vector2 v)
        {
            return new Vector3(v.x, v.y, 0);
        }
    }
}