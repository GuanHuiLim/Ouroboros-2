using System;
using System.Collections;

namespace Ouroboros
{
    public abstract class CustomYieldInstruction : IEnumerator
    {
        protected CustomYieldInstruction() { }

        public abstract bool keepWaiting { get; }
        public object Current => null;

        public bool MoveNext() => keepWaiting;

        public virtual void Reset() { }
    }

    public class WaitForSeconds : CustomYieldInstruction
    {
        public override bool keepWaiting { get { seconds -= Time.deltaTime; return seconds > 0; } }

        private float seconds;

        public WaitForSeconds(float seconds)
        {
            this.seconds = seconds;
        }
    }

    public class WaitUntil : CustomYieldInstruction
    {
        public override bool keepWaiting => !predicate();

        private Func<bool> predicate;

        public WaitUntil(Func<bool> predicate)
        {
            this.predicate = predicate;
        }
    }

    public class WaitWhile : CustomYieldInstruction
    {
        public override bool keepWaiting => predicate();

        private Func<bool> predicate;

        public WaitWhile(Func<bool> predicate)
        {
            this.predicate = predicate;
        }
    }
}
