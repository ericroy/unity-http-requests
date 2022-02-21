using System;
using System.Runtime.InteropServices;

namespace UnityHttpRequests
{

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct StringRef
    {
        // Memory layout must exactly correspond to the structs in the c header.  Do not reorder.
        private char* characters;
        public int Length { get; private set; }
        private int pad;

        public StringRef(string s)
        {
            fixed (char* p = s)
            {
                characters = p;
            }
            Length = s.Length;
            pad = 0;
        }

        public bool Equals(string s, bool caseSensitive = true)
        {
            if (s.Length != Length)
            {
                return false;
            }
            if (caseSensitive)
            {
                for (var i = 0; i < s.Length; ++i)
                {
                    if (s[i] != characters[i])
                        return false;
                }
            }
            else
            {
                for (var i = 0; i < s.Length; ++i)
                {
                    if (char.ToLowerInvariant(s[i]) != char.ToLowerInvariant(characters[i]))
                        return false;
                }
            }
            return true;
        }

        public bool StartsWith(string prefix, bool caseSensitive = true)
        {
            if (prefix.Length > Length)
            {
                return false;
            }
            if (caseSensitive)
            {
                for (var i = 0; i < prefix.Length; ++i)
                {
                    if (prefix[i] != characters[i])
                        return false;
                }
            }
            else
            {
                for (var i = 0; i < prefix.Length; ++i)
                {
                    if (char.ToLowerInvariant(prefix[i]) != char.ToLowerInvariant(characters[i]))
                        return false;
                }
            }
            return true;
        }

        public override string ToString()
        {
            throw new NotImplementedException();
        }

        public string ToStringAlloc()
        {
            return Marshal.PtrToStringUni((IntPtr)characters, Length);
        }
    }

}