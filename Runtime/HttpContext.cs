using System;
using System.Collections.Generic;
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

            context = UHR_CreateHttpContext();
            if (context == Constants.HttpContextInvalid)
            {
                StringRef err;
                UHR_GetLastError(&err);
                throw new Exception("Failed to create HttpContext: " + err.ToStringAlloc());
            }
        }

        ~HttpContext()
        {
            if (context != IntPtr.Zero)
            {
                UHR_DestroyHttpContext(context);
                context = IntPtr.Zero;
            }
        }

        public int Get(string url, Dictionary<string, string> headers = null)
        {
            return StartRequest(url, Constants.Method.GET, headers, null, 0);
        }

        public int Post(string url, byte[] requestBody, int requestBodyLength, Dictionary<string, string> headers = null)
        {
            return StartRequest(url, Constants.Method.POST, headers, requestBody, requestBodyLength);
        }

        // Call this once per frame to tick the library and dispatch responses.
        public void Update()
        {
            int* toDelete = stackalloc int[responsesBuffer.Length];
            fixed (Response* pResponses = &responsesBuffer[0])
            {
                int count = 0;
                while ((count = UHR_Update(context, pResponses, responsesBuffer.Length)) == responsesBuffer.Length)
                {
                    for (var i = 0; i < count; ++i)
                    {
                        toDelete[i] = pResponses[i].RequestId;
                        try
                        {
                            RequestComplete?.Invoke(ref pResponses[i]);
                        }
                        catch (Exception)
                        {
                            // log
                        }
                    }
                    if (UHR_DestroyRequests(context, toDelete, count) < count)
                    {
                        StringRef err;
                        UHR_GetLastError(&err);
                        throw new Exception("Failed to destroy one or more requests: " + err.ToStringAlloc());
                    }
                }
            }
        }

        private unsafe int StartRequest(
            string url,
            Constants.Method method,
            Dictionary<string, string> headers,
            byte[] requestBody,
            int requestBodyLength
        )
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

            int rid = Constants.RequestIdInvalid;

            fixed (Header* pHeaders = headersBuffer)
            {
                fixed (byte* pRequestBody = requestBody)
                {
                    rid = UHR_CreateRequest(
                        context,
                        new StringRef(url),
                        (int)method,
                        pHeaders,
                        headersCount,
                        pRequestBody,
                        pRequestBody != null ? requestBodyLength : 0
                    );
                    if (rid == Constants.RequestIdInvalid)
                    {
                        StringRef err;
                        UHR_GetLastError(&err);
                        throw new Exception("Failed to create http request: " + err.ToStringAlloc());
                    }
                }
            }

            return rid;
        }

        #region NativeBindings

#if UNITY_IPHONE
        [DllImport ("__Internal")]
#else
        [DllImport("UnityHttpRequests")]
#endif
        extern static void UHR_GetLastError(StringRef* error_out);

#if UNITY_IPHONE
        [DllImport ("__Internal")]
#else
        [DllImport("UnityHttpRequests")]
#endif
        extern static IntPtr UHR_CreateHttpContext();

#if UNITY_IPHONE
        [DllImport ("__Internal")]
#else
        [DllImport("UnityHttpRequests")]
#endif
        extern static void UHR_DestroyHttpContext(IntPtr context);

#if UNITY_IPHONE
        [DllImport ("__Internal")]
#else
        [DllImport("UnityHttpRequests")]
#endif
        extern static int UHR_CreateRequest(
            IntPtr context,
            StringRef url,
            int method,
            Header* requestHeaders,
            int requestHeadersCount,
            byte* requestBody,
            int requestBodyLength
        );

#if UNITY_IPHONE
        [DllImport ("__Internal")]
#else
        [DllImport("UnityHttpRequests")]
#endif
        extern static int UHR_Update(IntPtr context, Response* resultsOut, int resultsOutCapacity);

#if UNITY_IPHONE
        [DllImport ("__Internal")]
#else
        [DllImport("UnityHttpRequests")]
#endif
        extern static int UHR_DestroyRequests(IntPtr context, int* requestIds, int requestIdsCount);

        #endregion
    }

}
