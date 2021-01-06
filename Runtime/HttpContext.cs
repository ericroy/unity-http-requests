using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace UnityHttpRequests
{

    public unsafe class HttpContext
    {
        public event RequestCompleteDelegate RequestComplete;
        public delegate void RequestCompleteDelegate(ref Response result);

        private Header[] headersBuffer;
        private Response[] responsesBuffer;
        private IntPtr context = IntPtr.Zero;

        public HttpContext(int responsesCapacity = 8, int headersCapacity = 8)
        {
            if (responsesCapacity < 1)
            {
                throw new Exception("Responses capacity must be at least 1");
            }
            responsesBuffer = new Response[responsesCapacity];

            if (headersCapacity < 1)
            {
                throw new Exception("Headers capacity must be at least 1");
            }
            headersBuffer = new Header[headersCapacity];

            var err = UHR_CreateHTTPContext(&context);
            if (err != Error.Ok)
            {
                throw new UHRException("Failed to create HttpContext: ", err);
            }
        }

        ~HttpContext()
        {
            if (context != IntPtr.Zero)
            {
                var err = UHR_DestroyHTTPContext(context);
                if (err != Error.Ok)
                {
                    throw new UHRException("Failed to destroy HttpContext: ", err);
                }
                context = IntPtr.Zero;
            }
        }

        public int Get(string url, Dictionary<string, string> headers = null)
        {
            return StartRequest(url, Method.GET, headers, null, 0);
        }

        public int Post(string url, byte[] requestBody, int requestBodyLength, Dictionary<string, string> headers = null)
        {
            return StartRequest(url, Method.POST, headers, requestBody, requestBodyLength);
        }

        // Call this once per frame to tick the library and dispatch responses.
        public void Update()
        {
            int* toDelete = stackalloc int[responsesBuffer.Length];
            fixed (Response* pResponses = &responsesBuffer[0])
            {
                int count = 0;
                do
                {
                    var err = UHR_Update(context, pResponses, responsesBuffer.Length, &count);
                    if (err != Error.Ok)
                    {
                        throw new UHRException("Failed to update http requests: ", err);
                    }

                    if (count > 0)
                    {
                        for (var i = 0; i < count; ++i)
                        {
                            toDelete[i] = pResponses[i].RequestId;
                            try
                            {
                                RequestComplete?.Invoke(ref pResponses[i]);
                            }
                            catch (Exception ex)
                            {
                                Debug.WriteLine("Failed to handled HTTP response: ", ex.Message);
                            }
                        }
                        var err = UHR_DestroyRequests(context, toDelete, count);
                        if (err != Error.Ok)
                        {
                            throw new UHRException("Failed to destroy http requests: ", err);
                        }
                    }

                    // Keep looping as long as there may be more
                } while (count == responsesBuffer.Length);
            }
        }

        private unsafe int StartRequest(string url, Method method, Dictionary<string, string> headers, byte[] requestBody, int requestBodyLength)
        {
            int headersCount = 0;
            if (headers != null)
            {
                headersCount = headers.Count;
                if (headersCount > headersBuffer.Length)
                {
                    headersBuffer = new Header[headersCount];
                }
                int i = 0;
                var enumer = headers.GetEnumerator();
                while (enumer.MoveNext())
                {
                    headersBuffer[i++] = new Header
                    {
                        Name = new StringRef(enumer.Current.Key),
                        Value = new StringRef(enumer.Current.Value),
                    };
                }
            }

            if (requestBody != null)
            {
                if (requestBodyLength < 0 || requestBodyLength > requestBody.Length)
                {
                    throw new ArgumentException("Request body length out of range");
                }
            }

            int rid = 0;

            fixed (Header* pHeaders = headersBuffer)
            {
                fixed (byte* pRequestBody = requestBody)
                {
                    var err = UHR_CreateRequest(
                        context,
                        new StringRef(url),
                        (int)method,
                        pHeaders,
                        headersCount,
                        pRequestBody,
                        pRequestBody != null ? requestBodyLength : 0,
                        &rid
                    );
                    if (err != Errors.Ok)
                    {
                        throw new UHRException("Failed to create http request: ", err);
                    }
                }
            }

            return rid;
        }

        #region NativeBindings
#if UNITY_IPHONE
        [DllImport ("__Internal")]
#else
        [DllImport("UnityHttpRequests", CallingConvention = CallingConvention.Cdecl)]
#endif
        extern static Error UHR_CreateHTTPContext(IntPtr* httpContextHandleOut);

#if UNITY_IPHONE
        [DllImport ("__Internal")]
#else
        [DllImport("UnityHttpRequests", CallingConvention = CallingConvention.Cdecl)]
#endif
        extern static Error UHR_DestroyHTTPContext(IntPtr httpContextHandle);

#if UNITY_IPHONE
        [DllImport ("__Internal")]
#else
        [DllImport("UnityHttpRequests", CallingConvention = CallingConvention.Cdecl)]
#endif
        extern static Error UHR_CreateRequest(
            IntPtr httpContextHandle,
            StringRef url,
            Method method,
            Header* headers,
            uint headersCount,
            byte* body,
            uint bodyLength,
            int* ridOut
        );

#if UNITY_IPHONE
        [DllImport ("__Internal")]
#else
        [DllImport("UnityHttpRequests", CallingConvention = CallingConvention.Cdecl)]
#endif
        extern static Error UHR_Update(
            IntPtr httpContextHandle,
            Response* responsesOut,
            uint responsesCapacity,
            uint* responseCountOut
        );

#if UNITY_IPHONE
        [DllImport ("__Internal")]
#else
        [DllImport("UnityHttpRequests", CallingConvention = CallingConvention.Cdecl)]
#endif
        extern static Error UHR_DestroyRequests(
            IntPtr httpContextHandle,
            int* requestIDs,
            uint requestIDsCount
        );

#endregion
    }

}
