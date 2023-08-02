using System;
using System.Runtime.InteropServices;

namespace Ouroboros
{
    public class LoadProgress
    {
        private float m_percent;

        public float percent { get => m_percent; }
        public bool isDone { get => m_percent >= 100.0f; }
    }
}