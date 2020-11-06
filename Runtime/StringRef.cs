using System;
using System.Runtime.InteropServices;

namespace HttpRequests
{

    // Memory layout must be exactly compatible with its counterpart in the c header
    public unsafe ref struct StringRef {
        public char* Characters;
        public int Length;

        StringRef(string s) {
            fixed (char* p = &s[0]) {
                Characters = p;
            }
            Length = s.Length;
        }

        public bool Equals(string s) {
            if (s.Length != Length) {
                return false;
            }
            for (var i = 0; i < s.Length; ++i) {
                if (s[i] != Characters[i])
                    return false;
                }
            }
            return true;
        }

        public string ToStringAlloc() {
            Marshal.PtrToStringUni(Characters, Length);
        }
    }

}