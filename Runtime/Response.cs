using System;
using System.Runtime.InteropServices;

namespace UnityHttpRequests
{

    // Memory layout must be exactly compatible with its counterpart in the c header
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct Response
    {
        public int RequestId { get; private set; }
        public int HttpStatus { get; private set; }
        public Header* Headers { get; private set; }
        public int HeadersCount { get; private set; }
        public byte* Body { get; private set; }
        public int BodyLength { get; private set; }

        public bool TryGetHeaderAlloc(string headerName, ref string headerValueOut)
        {
            var index = FindHeader(headerName);
            if (index == -1)
            {
                return false;
            }
            headerValueOut = Headers[index].Value.ToStringAlloc();
            return true;
        }

        public void GetHeaderAtIndexAlloc(int index, out string name, out string value)
        {
            if (index < 0 || index >= HeadersCount)
            {
                throw new IndexOutOfRangeException();
            }
            var h = Headers[index];
            name = h.Name.ToStringAlloc();
            value = h.Value.ToStringAlloc();
        }

        public int FindHeader(string headerName)
        {
            for (var i = 0; i < HeadersCount; ++i)
            {
                if (Headers[i].Name.Equals(headerName))
                {
                    return i;
                }
            }
            return -1;
        }

        public int CopyResponseBodyTo(byte[] buffer)
        {
            int count = Math.Min(BodyLength, buffer.Length);
            Marshal.Copy((IntPtr)Body, buffer, 0, count);
            return count;
        }
    }

}