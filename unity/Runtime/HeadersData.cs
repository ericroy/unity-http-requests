using System;
using System.Runtime.InteropServices;

namespace UnityHttpRequests
{

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct HeadersData
    {
        // Memory layout must exactly correspond to the structs in the c header.  Do not reorder.
        private Header* headers;
        public uint Count { get; private set; }
        private uint pad;
        
        // Header names are case insensitive.
        public int FindHeader(string headerName)
        {
            for (var i = 0; i < Count; ++i)
            {
                if (headers[i].Name.Equals(headerName, false))
                {
                    return i;
                }
            }
            return -1;
        }

        // Returns the value of the specified header
        public string GetHeaderAlloc(int headerIndex)
        {
            if (headerIndex < 0 || headerIndex >= Count)
            {
                throw new IndexOutOfRangeException();
            }
            return headers[headerIndex].Value.ToStringAlloc();
        }

        // Gets the name and value of the specified header and writes them to the out params
        public void GetHeaderAlloc(int headerIndex, out string headerName, out string headerValue)
        {
            if (headerIndex < 0 || headerIndex >= Count)
            {
                throw new IndexOutOfRangeException();
            }
            headerName = headers[headerIndex].Name.ToStringAlloc();
            headerValue = headers[headerIndex].Value.ToStringAlloc();
        }

        // Tries to get a header with the specified name, and writes it to the out param if it exists.
        // Returns true if successful, false otherwise.
        public bool TryGetHeaderAlloc(string headerName, out string valueOut)
        {
            int i = FindHeader(headerName);
            if (i == -1)
            {
                valueOut = null;
                return false;
            }
            valueOut = headers[i].Value.ToStringAlloc();
            return true;
        }

        public bool HeaderEquals(int headerIndex, string expectedValue, bool valueCaseSensitive = true)
        {
            if (headerIndex < 0 || headerIndex >= Count)
            {
                throw new IndexOutOfRangeException();
            }
            return headers[headerIndex].Value.Equals(expectedValue, valueCaseSensitive);
        }

        public bool HeaderEquals(string headerName, string expectedValue, bool valueCaseSensitive = true)
        {
            int i = FindHeader(headerName);
            if (i == -1)
            {
                return false;                
            }
            return headers[i].Value.Equals(expectedValue, valueCaseSensitive);
        }
    }

}