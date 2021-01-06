using System;
using System.Runtime.InteropServices;

namespace UnityHttpRequests
{

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct BodyData
    {
        // Memory layout must exactly correspond to the structs in the c header.  Do not reorder.
        public byte* body { get; private set; }
        public int Length { get; private set; }
        private int pad;

        public int CopyTo(byte[] buffer)
        {
            int count = Math.Min(Length, buffer.Length);
            Marshal.Copy((IntPtr)body, buffer, 0, count);
            return count;
        }
    }

}