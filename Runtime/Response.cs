using System;
using System.Runtime.InteropServices;

namespace UnityHttpRequests
{

    // Memory layout must be exactly compatible with its counterpart in the c header
    public unsafe struct Response {
        public int RequestId { get; private set; }
        public int HttpStatus { get; private set; }
        public Header* Headers { get; private set; }
        public int HeadersCount { get; private set; }
        public byte* Body { get; private set; }
        public int BodyLength { get; private set; }

        public string GetHeaderValueAlloc(string headerName) {
            var index = FindHeaderIndex(headerName);
            if (index == -1) {
                return "";
            }
            return Headers[index].Value.ToStringAlloc();
        }

        public int CopyResponseBodyTo(byte[] buffer) {
            int count = Math.Min(BodyLength, buffer.Length);
            Marshal.Copy((IntPtr)Body, buffer, 0, count);
            return count;
        }

        private int FindHeaderIndex(string headerName) {
            for (var i = 0; i < HeadersCount; ++i) {
                if (Headers[i].Name.Equals(headerName)) {
                    return i;
                }
            }
            return -1;
        }
    }

}