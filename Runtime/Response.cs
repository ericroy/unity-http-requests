using System;
using System.Runtime.InteropServices;

namespace HttpRequests
{

    // Memory layout must be exactly compatible with its counterpart in the c header
    public ref struct Response {
        public int RequestID { get; private set; };
        public int HttpStatus { get; private set; };
        public Header* ResponseHeaders { get; private set; };
        public int ResponseHeadersCount { get; private set; };
        public byte* ResponseBody { get; private set; };
        public int ResponseBodyLength { get; private set; };

        public string GetHeaderValueAlloc(string headerName) {
            var index = FindHeaderIndex(headerName);
            if (index == -1) {
                return "";
            }
            return responseHeaders[index].Value.ToStringAlloc();
        }

        public int CopyResponseBodyTo(byte[] buffer) {
            int count = Math.Min(responseBodyLength, buffer.Length);
            Marshal.Copy(responseBody, buffer, 0, count);
            return count;
        }

        private int FindHeaderIndex(string headerName) {
            for (var i = 0; i < responseHeadersCount; ++i) {
                if (reponseHeaders[i].Name.Equals(headerName)) {
                    return i;
                }
            }
            return -1;
        }
    }

}