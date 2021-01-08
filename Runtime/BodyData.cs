using System;
using System.Diagnostics;
using System.Text;
using System.Runtime.InteropServices;

namespace UnityHttpRequests
{

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct BodyData
    {
        // Memory layout must exactly correspond to the structs in the c header.  Do not reorder.
        public byte* body { get; private set; }
        public uint Length { get; private set; }
        private uint pad;

        public int CopyTo(byte[] buffer)
        {
            uint count = Math.Min(Length, (uint)buffer.Length);
            if (count > int.MaxValue)
            {
                // Response body length overflows int32, truncate!
                count = int.MaxValue;
                Debug.WriteLine("Response body length overflowed int, truncating");
            }

            Marshal.Copy((IntPtr)body, buffer, 0, (int)count);
            return (int)count;
        }

        public override string ToString()
        {
            throw new NotImplementedException();
        }

        public string ToStringAlloc()
        {
            uint length = Length;
            if (length > int.MaxValue)
            {
                // Response body length overflows int32, truncate!
                length = int.MaxValue;
                Debug.WriteLine("Response body length overflowed int, truncating");
            }

            byte[] buffer = new byte[(int)length];
            CopyTo(buffer);
            return Encoding.UTF8.GetString(buffer);
        }
    }

}