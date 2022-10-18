namespace Ouroboros
{
    public struct Vector3
    {
        public float x;
        public float y;
        public float z;

        public Vector3(float x, float y)
        {
            this.x = x;
            this.y = y;
            this.z = 0;
        }
        public Vector3(float x, float y, float z)
        {
            this.x = x;
            this.y = y;
            this.z = z;
        }

        public float this[int index]
        {
            get
            {
                switch (index)
                {
                    case 0: return x;
                    case 1: return y;
                    case 2: return z;
                    default: return 0;
                }
            }
            set
            {
                switch (index)
                {
                    case 0: x = value; break;
                    case 1: y = value; break;
                    case 2: z = value; break;
                }
            }
        }
        public static Vector3 right { get { return new Vector3(1.0f, 0.0f, 0.0f); } }
        public static Vector3 left { get { return new Vector3(-1.0f, 0.0f, 0.0f); } }
        public static Vector3 up { get { return new Vector3(0.0f, 1.0f, 0.0f); } }
        public static Vector3 down { get { return new Vector3(0.0f, -1.0f, 0.0f); } }
        public static Vector3 back { get { return new Vector3(0.0f, 0.0f, -1.0f); } }
        public static Vector3 forward { get { return new Vector3(0.0f, 0.0f, 1.0f); } }
        public static Vector3 one { get { return new Vector3(1.0f, 1.0f, 1.0f); } }
        public static Vector3 zero { get { return new Vector3(0.0f, 0.0f, 0.0f); } }
        public static Vector3 negativeInfinity { get { return new Vector3(float.NegativeInfinity, float.NegativeInfinity, float.NegativeInfinity); } }
        public static Vector3 positiveInfinity { get { return new Vector3(float.PositiveInfinity, float.PositiveInfinity, float.PositiveInfinity); } }

        public Vector3 normalized
        {
            get
            {
                Vector3 result = new Vector3(x, y, z);
                return result / magnitude;
            }
        }
        public float magnitude { get { return Mathf.Sqrt((x * x) + (y * y) + (z * z)); } }
        public float sqrMagnitude { get { return (x * x) + (y * y) + (z * z); } }

        public static float Angle(Vector3 from, Vector3 to)
        {
            return Mathf.Acos(Dot(from, to) / (from.magnitude * to.magnitude)) * Mathf.Rad2Deg;
        }
        public static Vector3 ClampMagnitude(Vector3 vector, float maxLength)
        {
            float length = vector.magnitude;
            if (length > maxLength)
            {
                vector /= length;
                vector *= maxLength;
            }
            return vector;
        }
        public static Vector3 Cross(Vector3 lhs, Vector3 rhs)
        {
            return new Vector3
            (
                (lhs.y * rhs.z) - (rhs.y * lhs.z),
                (rhs.x * lhs.z) - (lhs.x * rhs.z),
                (lhs.x * rhs.y) - (rhs.x * lhs.y)
            );
        }
        public static float Distance(Vector3 a, Vector3 b)
        {
            Vector3 diff = a - b;
            return diff.magnitude;
        }
        public static float Dot(Vector3 lhs, Vector3 rhs)
        {
            return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z);
        }
        public static Vector3 Lerp(Vector3 a, Vector3 b, float t)
        {
            return new Vector3(Mathf.Lerp(a.x, b.x, t), Mathf.Lerp(a.y, b.y, t), Mathf.Lerp(a.z, b.z, t));
        }
        public static Vector3 LerpUnclamped(Vector3 a, Vector3 b, float t)
        {
            return new Vector3(Mathf.LerpUnclamped(a.x, b.x, t), Mathf.LerpUnclamped(a.y, b.y, t), Mathf.LerpUnclamped(a.z, b.z, t));
        }
        public static float Magnitude(Vector3 vector)
        {
            return Mathf.Sqrt((vector.x * vector.x) + (vector.y * vector.y) + (vector.z * vector.z));
        }
        public static Vector3 Max(Vector3 lhs, Vector3 rhs)
        {
            return new Vector3(Mathf.Max(lhs.x, rhs.x), Mathf.Max(lhs.y, rhs.y), Mathf.Max(lhs.z, rhs.z));
        }
        public static Vector3 Min(Vector3 lhs, Vector3 rhs)
        {
            return new Vector3(Mathf.Min(lhs.x, rhs.x), Mathf.Min(lhs.y, rhs.y), Mathf.Min(lhs.z, rhs.z));
        }
        public static Vector3 MoveTowards(Vector3 current, Vector3 target, float maxDistanceDelta)
        {
            Vector3 result = target;
            if (maxDistanceDelta < 0 || (target - current).sqrMagnitude > maxDistanceDelta * maxDistanceDelta)
                result = current + ((target - current).normalized * maxDistanceDelta);
            return result;
        }
        public static Vector3 Normalize(Vector3 value)
        {
            Vector3 result = new Vector3(value.x, value.y, value.z);
            return result / value.magnitude;
        }
        public static void OrthoNormalize(ref Vector3 normal, ref Vector3 tangent, ref Vector3 binormal)
        {
            normal.Normalize();
            tangent = tangent - Project(tangent, normal);
            tangent.Normalize();
            binormal = binormal - Project(binormal, normal) - Project(binormal, tangent);
            binormal.Normalize();
        }
        public static void OrthoNormalize(ref Vector3 normal, ref Vector3 tangent)
        {
            normal.Normalize();
            tangent = tangent - Project(tangent, normal);
            tangent.Normalize();
        }
        public static Vector3 Project(Vector3 vector, Vector3 onNormal)
        {
            return (Dot(vector, onNormal) / Dot(onNormal, onNormal)) * onNormal;
        }
        public static Vector3 ProjectOnPlane(Vector3 vector, Vector3 planeNormal)
        {
            return vector - Project(vector, planeNormal);
        }
        public static Vector3 Reflect(Vector3 inDirection, Vector3 inNormal)
        {
            return inDirection - (2.0f * Project(inDirection, inNormal));
        }
        //public static Vector3 RotateTowards(Vector3 current, Vector3 target, float maxRadiansDelta, float maxMagnitudeDelta);
        public static Vector3 Scale(Vector3 a, Vector3 b)
        {
            Vector3 result = new Vector3(a.x, a.y, a.z);
            result.x *= b.x;
            result.y *= b.y;
            result.z *= b.z;
            return result;
        }
        public static float SignedAngle(Vector3 from, Vector3 to, Vector3 axis)
        {
            int dir = (Dot(Cross(from, to), axis) > 0) ? 1 : -1;
            return Angle(from, to) * dir;
        }
        //public static Vector3 Slerp(Vector3 a, Vector3 b, float t);
        //public static Vector3 SlerpUnclamped(Vector3 a, Vector3 b, float t);
        //public static Vector3 SmoothDamp(Vector3 current, Vector3 target, ref Vector3 currentVelocity, float smoothTime);
        //public static Vector3 SmoothDamp(Vector3 current, Vector3 target, ref Vector3 currentVelocity, float smoothTime, float maxSpeed);
        //public static Vector3 SmoothDamp(Vector3 current, Vector3 target, ref Vector3 currentVelocity, float smoothTime, [DefaultValue("Mathf.Infinity")] float maxSpeed, [DefaultValue("Time.deltaTime")] float deltaTime);
        public static float SqrMagnitude(Vector3 vector)
        {
            return (vector.x * vector.x) + (vector.y * vector.y) + (vector.z * vector.z);
        }
        public override bool Equals(object other)
        {
            return base.Equals(other);
        }
        public bool Equals(Vector3 other)
        {
            return Mathf.Approximately(x, other.x) && Mathf.Approximately(y, other.y) && Mathf.Approximately(z, other.z);
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
            z /= length;
        }
        public void Scale(Vector3 scale)
        {
            x *= scale.x;
            y *= scale.y;
            z *= scale.z;
        }
        public void Set(float newX, float newY, float newZ)
        {
            x = newX;
            y = newY;
            z = newZ;
        }
        public override string ToString()
        {
            return "(" + x + ", " + y + ", " + z + ")";
        }

        public static Vector3 operator +(Vector3 a, Vector3 b)
        {
            return new Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
        }
        public static Vector3 operator -(Vector3 a)
        {
            return new Vector3(-a.x, -a.y, -a.z);
        }
        public static Vector3 operator -(Vector3 a, Vector3 b)
        {
            return new Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
        }
        public static Vector3 operator *(float d, Vector3 a)
        {
            return new Vector3(a.x * d, a.y * d, a.z * d);
        }
        public static Vector3 operator *(Vector3 a, float d)
        {
            return new Vector3(a.x * d, a.y * d, a.z * d);
        }
        public static Vector3 operator /(Vector3 a, float d)
        {
            return new Vector3(a.x / d, a.y / d, a.z / d);
        }
        public static bool operator ==(Vector3 lhs, Vector3 rhs)
        {
            return Mathf.Approximately(lhs.x, rhs.x) && Mathf.Approximately(lhs.y, rhs.y) && Mathf.Approximately(lhs.z, rhs.z);
        }
        public static bool operator !=(Vector3 lhs, Vector3 rhs)
        {
            return !(lhs == rhs);
        }
    }
}