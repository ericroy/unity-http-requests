using System;

namespace UnityHttpRequests
{

    // Values must match constants in c header
    public static class Constants {
        public static readonly IntPtr HttpContextInvalid = IntPtr.Zero;
        public const int RequestIdInvalid = 0;

        public enum Method : int {
            GET = 0,
            POST = 1,
        }
    }

}

