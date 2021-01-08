using System;
using System.Runtime.InteropServices;

namespace UnityHttpRequests
{

    public unsafe class UHRException : Exception
    {
        public UHRException(string prefix, Error err) : base(prefix + Format(err))
        {
        }

        private static string Format(Error err)
        {
            StringRef errorMessage;
            Error fmtErr = UHR_ErrorToString(err, &errorMessage);
            if (fmtErr != Error.Ok) {
                return "???";
            }
            return errorMessage.ToStringAlloc();
        }

    #if UNITY_IPHONE
        [DllImport ("__Internal")]
    #else
        [DllImport("UnityHttpRequests", CallingConvention = CallingConvention.Cdecl)]
    #endif
        extern static Error UHR_ErrorToString(Error err, StringRef* errorMessageOut);
    }

}
