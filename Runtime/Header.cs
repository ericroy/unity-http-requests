using System;
using System.Runtime.InteropServices;

namespace UnityHttpRequests
{

    // Memory layout must be exactly compatible with its counterpart in the c header
    [StructLayout(LayoutKind.Sequential)]
    public struct Header
    {
        public StringRef Name;
        public StringRef Value;
    }

}