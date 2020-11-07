using System;
using System.Text;
using System.Collections.Generic;

namespace HttpRequests
{
    public class HttpContext {
        Header[] headersScratch;
        Response[] responsesScratch;
        IntPtr context = 0;
        
        public event RequestCompleteDelegate RequestComplete;

        public delegate RequestCompleteDelegate(ref Response result);

        public unsafe HttpContext(int responsesCapacity = 8, int headersCapacity = 8) {
            if (responsesCapacity < 1) {
                throw new Exception("Responses capacity must be at least 1");
            }
            responsesScratch = new Response[responsesCapacity];

            if (headersCapacity < 1) {
                throw new Exception("Headers capacity must be at least 1");
            }
            headersScratch = new Header[headersCapacity];

            context = UHR_CreateContext();
            if (context == Constants.HttpContextInvalid) {
                StringRef err;
                fixed (StringRef* pErr = &err) {
                    UHR_GetLastError(pErr);
                }
                throw new Exception("Failed to create HttpContext: " + err.ToStringAlloc());
            }
        }

        public ~HttpContext() {
            if (context != 0) {
                UHR_DestroyContext(context);
                context = 0;
            }
        }

        public int Get(string url, Dictionary<string, string> headers = null) {
            return StartRequest(url, Method.GET, headers, null, 0);
        }

        public int Post(string url, byte[] requestBody, int requestBodyLength, Dictionary<string, string> headers = null) {
            return StartRequest(url, Method.POST, headers, requestBody, requestBodyLength);
        }

        // Call this once per frame to tick the library and dispatch responses.
        public unsafe void Update() {
            int* toDelete = stackalloc int[responsesScratch.Length];
            fixed (Response* pResponses = &responsesScratch[0]) {
                while ((int count = UHR_Update(context, pResponses, responsesScratch.Length)) != 0) {
                    for (var i = 0; i < count; ++i) {
                        toDelete[i] = pResponses[i].requestId;
                        try {
                            RequestComplete?.Invoke(ref pResponses[i]);
                        } catch (Exception) {
                            // log
                        }
                    }
                    if (UHR_DestroyRequests(context, toDelete, count) == -1) {
                        StringRef err;
                        fixed (StringRef* pErr = &err) {
                            UHR_GetLastError(pErr);
                        }
                        throw new Exception("Failed to destroy requests: " + err.ToStringAlloc());
                    }
                }
            }
        }

        private unsafe int StartRequest(
            string url,
            Method method,
            Dictionary<string, string> headers,
            byte[] requestBody,
            int requestBodyLength
        ) {
            int headersCount = 0;
            if (headers) {
                headersCount = headers.Count;
                if (headersCount > headersScratch.Length) {
                    headersScratch = new Header[headersCount];
                }
                for (var i = 0; i < headersCount; ++i) {
                    var pair = dictionary.ElementAt(i);
                    headers[i] = new Header{
                        Name = StringRef(pair.Key),
                        Value = StringRef(pair.Value),
                    };
                }
            }

            if (requestBody) {
                if (requestBodyLength < 0 || requestBodyLength > requestBody.Length) {
                    throw new ArgumentException("Request body length out of range");
                }
            }

            fixed (Header* pHeaders = &headers[0]) {
                fixed (byte* pRequestBody = (requestBody == null || requestBodyLength == 0) ? null : &requestBody[0]) {
                    int requestId = UHR_CreateJob(
                        context,
                        StringRef(url),
                        method,
                        pHeaders,
                        headersCount,
                        pRequestBody,
                        pRequestBody ? requestBodyLength : 0,
                    )
                    if (requestId == Constants.RequestIdInvalid) {
                        StringRef err;
                        fixed (StringRef* pErr = &err) {
                            UHR_GetLastError(pErr);
                        }
                        throw new Exception("Failed to create http request: " + err.ToStringAlloc());
                    }
                }
            }
        }

        #region NativeBindings

        #if UNITY_IPHONE
        [DllImport ("__Internal")]
        #else
        [DllImport ("UnityHttpRequests")]
        #endif
        extern static void UHR_GetLastError(StringRef *error_out);

        #if UNITY_IPHONE
        [DllImport ("__Internal")]
        #else
        [DllImport ("UnityHttpRequests")]
        #endif
        extern static IntPtr UHR_CreateHttpContext();

        #if UNITY_IPHONE
        [DllImport ("__Internal")]
        #else
        [DllImport ("UnityHttpRequests")]
        #endif
        extern static void UHR_DestroyHttpContext(IntPtr context);

        #if UNITY_IPHONE
        [DllImport ("__Internal")]
        #else
        [DllImport ("UnityHttpRequests")]
        #endif
        extern static int UHR_CreateRequest(
            IntPtr context,
            StringRef url,
            int method,
            Header* requestHeaders,
            int requestHeadersCount,
            byte* requestBody,
            int requestBodyLength,
        );

        #if UNITY_IPHONE
        [DllImport ("__Internal")]
        #else
        [DllImport ("UnityHttpRequests")]
        #endif
        extern static int UHR_Update(IntPtr context, Response* resultsOut, int resultsOutCapacity);
        
        #if UNITY_IPHONE
        [DllImport ("__Internal")]
        #else
        [DllImport ("UnityHttpRequests")]
        #endif
        extern static void UHR_DestroyRequests(IntPtr context, int* requestIds, int requestIdsCount);

        #endregion
    }

}
