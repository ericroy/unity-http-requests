using System;
using System.Runtime.InteropServices;

namespace UnityHttpRequests
{

    // Memory layout must be exactly compatible with its counterpart in the c header
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct StringRef
    {
        public char* Characters;
        public int Length;

        public StringRef(string s)
        {
            fixed (char* p = s)
            {
                Characters = p;
            }
            Length = s.Length;
        }

        public bool Equals(string s)
        {
            if (s.Length != Length)
            {
                return false;
            }
            for (var i = 0; i < s.Length; ++i)
            {
                if (s[i] != Characters[i])
                    return false;
            }
            return true;
        }

        public string ToStringAlloc()
        {
            return Marshal.PtrToStringUni((IntPtr)Characters, Length);
        }
    }

}