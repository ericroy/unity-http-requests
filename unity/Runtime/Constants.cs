using System;

namespace UnityHttpRequests
{

    // Values must match constants in c header
    public enum Error : uint
    {
        Ok = 0,
        InvalidSession = 1,
        MissingRequiredParameter = 2,
        InvalidHTTPMethod = 3,
        FailedToCreateRequest = 4,
        UnknownErrorCode = 5,
        FailedToCreateSession = 6,
        FailedToUpdateSession = 7,
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
    #if (UNITY_EDITOR_WIN || UNITY_STANDALONE_WIN)
        #if UNITY_64
        public const string Name = "UnityHttpRequests-amd64";
        #else
        #error "Windowns 32 bit not supported"
        #endif
    #elif (UNITY_EDITOR_OSX || UNITY_STANDALONE_OSX)
        #if UNITY_64
        public const string Name = "UnityHttpRequests-amd64";
        #else
        #error "OSX 32 bit not supported"
        #endif
    #elif (UNITY_EDITOR_LINUX || UNITY_STANDALONE_LINUX)
        #if UNITY_64
        public const string Name = "UnityHttpRequests-amd64";
        #else
        #error "Linux 32 bit not supported"
        #endif
    #elif UNITY_IPHONE
        public const string Name = "__Internal";
    #elif UNITY_ANDROID
        #if UNITY_64
        #error "Android arm 64 bit not yet supported..."
        #else
        public const string Name = "UnityHttpRequests-armeabi-v7a";
        #endif
    #endif
    }

}

