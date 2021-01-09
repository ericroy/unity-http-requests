using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace UnityHttpRequests
{

    public unsafe class HttpContext : IDisposable
    {
        public event RequestCompleteDelegate RequestComplete;
        public delegate void RequestCompleteDelegate(ref Response res);

        private Header[] headersBuffer;
        private Response[] responsesBuffer;
        private IntPtr context = IntPtr.Zero;
        private bool disposed = false;

        public HttpContext(int responsesCapacity = 8, int headersCapacity = 16)
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

            fixed (IntPtr* pContext = &context)
            {
                var err = UHR_CreateHTTPContext(pContext);
                if (err != Error.Ok)
                {
                    throw new UHRException("Failed to create HttpContext: ", err);
                }
            }
        }

        ~HttpContext() {}

        public uint Get(string url, Dictionary<string, string> headers = null)
        {
            return StartRequest(url, Method.Get, headers, null, 0);
        }

        public uint Post(string url, byte[] requestBody, int requestBodyLength, Dictionary<string, string> headers = null)
        {
            return StartRequest(url, Method.Post, headers, requestBody, requestBodyLength);
        }

        // Call this once per frame to tick the library and dispatch responses.
        public void Update()
        {
            uint* toDelete = stackalloc uint[responsesBuffer.Length];
            fixed (Response* pResponses = &responsesBuffer[0])
            {
                uint count = 0;
                do
                {
                    var err = UHR_Update(context, pResponses, (uint)responsesBuffer.Length, &count);
                    if (err != Error.Ok)
                    {
                        throw new UHRException("Failed to update http requests: ", err);
                    }

                    if (count > 0)
                    {
                        for (uint i = 0; i < count; ++i)
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
                        err = UHR_DestroyRequests(context, toDelete, count);
                        if (err != Error.Ok)
                        {
                            throw new UHRException("Failed to destroy http requests: ", err);
                        }
                    }

                    // Keep looping as long as there may be more
                } while (count == responsesBuffer.Length);
            }
        }

        public void Dispose()
        {
            // Dispose of unmanaged resources.
            Dispose(true);
            // Suppress finalization.
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Dispose all used resources.
        /// </summary>
        /// <param name="disposing">Indicates the source call to dispose.</param>
        private void Dispose(bool disposing)
        {
            if (this.disposed)
            {
                return;
            }

            if (disposing)
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

                this.disposed = true;
            }
        }

        private unsafe uint StartRequest(string url, Method method, Dictionary<string, string> headers, byte[] requestBody, int requestBodyLength)
        {
            uint headersCount = 0;
            if (headers != null)
            {
                headersCount = (uint)headers.Count;
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

            uint rid = 0;

            fixed (Header* pHeaders = headersBuffer)
            {
                fixed (byte* pRequestBody = requestBody)
                {
                    var err = UHR_CreateRequest(
                        context,
                        new StringRef(url),
                        method,
                        pHeaders,
                        headersCount,
                        pRequestBody,
                        pRequestBody != null ? (uint)requestBodyLength : 0,
                        &rid
                    );
                    if (err != Error.Ok)
                    {
                        throw new UHRException("Failed to create http request: ", err);
                    }
                }
            }

            return rid;
        }

        #region NativeBindings

        [DllImport(NativeLibrary.Name, CallingConvention = CallingConvention.Cdecl)]
        extern static Error UHR_CreateHTTPContext(IntPtr* httpContextHandleOut);

        [DllImport(NativeLibrary.Name, CallingConvention = CallingConvention.Cdecl)]
        extern static Error UHR_DestroyHTTPContext(IntPtr httpContextHandle);

        [DllImport(NativeLibrary.Name, CallingConvention = CallingConvention.Cdecl)]
        extern static Error UHR_CreateRequest(
            IntPtr httpContextHandle,
            StringRef url,
            Method method,
            Header* headers,
            uint headersCount,
            byte* body,
            uint bodyLength,
            uint* ridOut
        );

        [DllImport(NativeLibrary.Name, CallingConvention = CallingConvention.Cdecl)]
        extern static Error UHR_Update(
            IntPtr httpContextHandle,
            Response* responsesOut,
            uint responsesCapacity,
            uint* responseCountOut
        );

        [DllImport(NativeLibrary.Name, CallingConvention = CallingConvention.Cdecl)]
        extern static Error UHR_DestroyRequests(
            IntPtr httpContextHandle,
            uint* requestIDs,
            uint requestIDsCount
        );

#endregion
    }

}
