using System;

namespace UnityHttpRequests
{

    // Values must match constants in c header
    public static class Constants
    {
        public enum Error : int
        {
            Ok = 0,
            InvalidContext = -1,
            MissingRequiredParameter = -2,
            InvalidHTTPMethod = -3,
            FailedToCreateRequest = -4,
            UnknownErrorCode = -5,
        }
        
        public enum Method : int
        {
            Get = 0,
            Head = 1,
            Post = 2,
            Put = 3,
            Patch = 4,
            Delete = 5,
            Connect = 6,
            Options = 7,
            Trace = 8,
        }
    }

}

