using System;
using System.Runtime.InteropServices;

namespace UnityHttpRequests
{

    [StructLayout(LayoutKind.Sequential)]
    public struct Header
    {
        // Memory layout must exactly correspond to the structs in the c header.  Do not reorder.
        public StringRef Name;
        public StringRef Value;
    }

}