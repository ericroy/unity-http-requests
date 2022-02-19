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
        private const string NameWin64 = "uhr-windows.x86_64";
        private const string NameLinux64 = "uhr-linux.x86_64";
        private const string NameMac = "uhr-mac.fat";
        private const string NameIOS = "__Internal";
        private const string NameAndroid = "uhr-android.armeabi-v7a";

    #if UHR_TESTS
        #if UHR_TESTS_WINDOWS
            public const string Name = "../../Assets/Plugins/x86_64/" + NameWin64;
        #elif UHR_TESTS_LINUX
            public const string Name = "../../Assets/Plugins/x86_64/" + NameLinux64;
        #elif UHR_TESTS_MAC
            public const string Name = "../../Assets/Plugins/" + NameMac;
        #elif UHR_TESTS_ANDROID
            public const string Name = "../../Assets/Plugins/Android/" + NameAndroid;
        #elif UHR_TESTS_IOS
            public const string Name = NameIOS;
        #else
            #error "Unsupported test platform"
        #endif
    #else
        #if (UNITY_EDITOR_WIN || UNITY_STANDALONE_WIN)
            #if UNITY_64
            public const string Name = NameWin64;
            #else
            #error "Windows 32 bit not supported"
            #endif
        #elif (UNITY_EDITOR_OSX || UNITY_STANDALONE_OSX)
            #if UNITY_64
            public const string Name = NameMac;
            #else
            #error "OSX 32 bit not supported"
            #endif
        #elif (UNITY_EDITOR_LINUX || UNITY_STANDALONE_LINUX)
            #if UNITY_64
            public const string Name = NameLinux64;
            #else
            #error "Linux 32 bit not supported"
            #endif
        #elif UNITY_IPHONE
            public const string Name = NameIOS;
        #elif UNITY_ANDROID
            #if UNITY_64
            #error "Android arm 64 bit not yet supported..."
            #else
            public const string Name = NameAndroid;
            #endif
        #else
            #error "No UHR plugin for this platform"
        #endif
    #endif
    }

}

