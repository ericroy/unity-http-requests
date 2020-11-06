using System;

namespace HttpRequests
{

    // Memory layout must be exactly compatible with its counterpart in the c header
    public ref struct Header {
        public StringRef Name;
        public StringRef Value;
    }

}