namespace Ouroboros
{
    [System.AttributeUsage(System.AttributeTargets.Field, AllowMultiple = false, Inherited = true)]
    public class NonSerialized : System.Attribute
    {

    }

    [System.AttributeUsage(System.AttributeTargets.Field, AllowMultiple = false, Inherited = true)]
    public class SerializeField : System.Attribute
    {

    }

    [System.AttributeUsage(System.AttributeTargets.Field, AllowMultiple = false, Inherited = true)]
    public class HeaderAttribute : System.Attribute
    {
        private string Name;
        public HeaderAttribute(string name)
        {
            Name = name;
        }
    }
}