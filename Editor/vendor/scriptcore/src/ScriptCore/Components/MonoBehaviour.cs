using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Ouroboros
{
    public class Coroutine
    {
        public Coroutine(IEnumerator enumerator)
        {
            Enumerator = enumerator;
        }

        public IEnumerator Enumerator { get; private set; }
    }

    public class MonoBehaviour : Component
    {
        protected MonoBehaviour()
        {

        }

        [DllImport("__Internal")] private static extern bool CheckEntityExists(UInt32 sceneID, UInt64 uuid);

        public static bool operator ==(MonoBehaviour lhs, MonoBehaviour rhs)
        {
            if (ReferenceEquals(lhs, null) && ReferenceEquals(rhs, null)) // lhs is null, and rhs is null
                return true;

            if (!ReferenceEquals(lhs, null) && ReferenceEquals(rhs, null)) // lhs is not null, but rhs is null
                return lhs.gameObject == null || !ReferenceEquals(lhs, lhs.GetComponent(lhs.GetType()));

            if (ReferenceEquals(lhs, null) && !ReferenceEquals(rhs, null)) // lhs is null, but rhs is not null
                return rhs.gameObject == null || !ReferenceEquals(rhs, rhs.GetComponent(rhs.GetType()));

            return ReferenceEquals(lhs, rhs);
        }

        public static bool operator !=(MonoBehaviour lhs, MonoBehaviour rhs)
        {
            return !(lhs == rhs);
        }

        public override bool Equals(object obj)
        {
            return base.Equals(obj);
        }

        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        [DllImport("__Internal")] private static extern void SetScriptEnabled(UInt32 sceneID, UInt64 uuid, string name_space, string name, bool enabled);
        [DllImport("__Internal")] private static extern bool CheckScriptEnabled(UInt32 sceneID, UInt64 uuid, string name_space, string name);

        public bool enabled
        {
            get { return CheckScriptEnabled(gameObject.scene, gameObject.GetInstanceID(), GetType().Namespace ?? "", GetType().Name); ; }
            set { SetScriptEnabled(gameObject.scene, gameObject.GetInstanceID(), GetType().Namespace ?? "", GetType().Name, value); }
        }

        #region Coroutines

        private List<Coroutine> coroutines = new List<Coroutine>();
        private List<Coroutine> coroutineStopList = new List<Coroutine>();

        public Coroutine StartCoroutine(IEnumerator coroutine)
        {
            Coroutine cor = new Coroutine(coroutine);
            coroutines.Add(cor);
            return cor;
        }

        public void StopCoroutine(Coroutine coroutine)
        {
            coroutineStopList.Add(coroutine);
        }

        public void StopAllCoroutines()
        {
            coroutineStopList.AddRange(coroutines);
        }

        private void TickCoroutines()
        {
            for (int i = coroutines.Count - 1; i >= 0; --i)
            {
                if (!TickCoroutine(coroutines[i].Enumerator))
                    coroutines.RemoveAt(i);
            }
            foreach (var coroutine in coroutineStopList)
            {
                if (coroutines.Contains(coroutine))
                    coroutines.Remove(coroutine);
            }
            coroutineStopList.Clear();
        }

        private bool TickCoroutine(IEnumerator coroutine)
        {
            var curr = coroutine.Current;
            if (curr != null && curr is IEnumerator)
            {
                bool subresult = TickCoroutine((IEnumerator)curr);
                if (subresult)
                    return true;
            }
            bool result = coroutine.MoveNext();
            return result;
        }

        #endregion
    }
}