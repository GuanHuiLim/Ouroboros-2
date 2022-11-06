using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Ouroboros
{
    [StructLayout(LayoutKind.Sequential)]
    public struct ContactPoint
    {
        private Vector3 m_normal;
        private Vector3 m_point;
        private Vector3 m_impulse;

        public Vector3 point { get => m_point; }
        public Vector3 normal { get => m_normal; }
        public Vector3 impulse { get => m_impulse; }
    }

    [StructLayout(LayoutKind.Sequential)]
    public class Collision
    {
        private GameObject m_gameObject;
        private Rigidbody m_rigidbody;
        private Collider m_collider;

        private ContactPoint[] m_contacts;

        public GameObject gameObject { get => m_gameObject; }
        public Rigidbody rigidbody { get => m_rigidbody; }
        public Collider collider { get => m_collider; }
        public int contactCount { get => m_contacts.Length; }

        public void Test()
        {
            Debug.Log(m_contacts);
        }

        public ContactPoint GetContact(int index)
        {
            return m_contacts[index];
        }

        public int GetContacts(ContactPoint[] contacts)
        {
            contacts = m_contacts;
            return m_contacts.Length;
        }

        public int GetContacts(List<ContactPoint> contacts)
        {
            contacts = new List<ContactPoint>(m_contacts);
            return m_contacts.Length;
        }
    }
}