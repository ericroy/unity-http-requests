using System;

namespace HttpRequests
{

    // Values must match constants in c header
    public static class Constants {
        public const IntPtr HttpContextInvalid = IntPtr.Zero;
        public const int RequestIdInvalid = 0;

        public enum Method : int {
            GET = 0,
            POST = 1,
        }
    }

}

