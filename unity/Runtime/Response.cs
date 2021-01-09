using System;
using System.Runtime.InteropServices;

namespace UnityHttpRequests
{

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct Response
    {
        // Memory layout must exactly correspond to the structs in the c header.  Do not reorder.
        public uint RequestId { get; private set; }
        public uint HttpStatus { get; private set; }
        public HeadersData Headers;
        public BodyData Body;
    }

}