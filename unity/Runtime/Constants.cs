using System;

namespace UnityHttpRequests
{

    // Values must match constants in c header
    public enum Error : uint
    {
        Ok = 0,
        InvalidContext = 1,
        MissingRequiredParameter = 2,
        InvalidHTTPMethod = 3,
        FailedToCreateRequest = 4,
        UnknownErrorCode = 5,
    }

    // Values must match constants in c header
    public enum Method : uint
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

    class NativeLibrary
    {
        #if UNITY_IPHONE
        public const string Name = "__Internal";
        #else
        public const string Name = "UnityHttpRequests";
        #endif
    }

}

