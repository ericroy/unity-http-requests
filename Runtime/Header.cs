using System;

namespace UnityHttpRequests
{

    // Memory layout must be exactly compatible with its counterpart in the c header
    public struct Header {
        public StringRef Name;
        public StringRef Value;
    }

}