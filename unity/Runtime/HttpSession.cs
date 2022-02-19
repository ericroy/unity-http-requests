using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
#if UHR_TESTS
using NUnit.Framework;
#endif

namespace UnityHttpRequests
{

	public unsafe class HttpSession : IDisposable
	{
		public event RequestCompleteDelegate RequestComplete;
		public delegate void RequestCompleteDelegate(ref Response res);

		private Header[] headersBuffer;
		private Response[] responsesBuffer;
		private IntPtr session = IntPtr.Zero;
		private bool disposed = false;
		
		private delegate void LoggingCallback(StringRef msg, IntPtr userData);
		private static readonly LoggingCallback logMessageDelegate = LogMessage;
		
		static HttpSession()
		{
			#if DEBUG
			UHR_SetLoggingCallback(logMessageDelegate, IntPtr.Zero);
			#endif
		}

		public HttpSession(int responsesCapacity = 8, int headersCapacity = 16)
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

			fixed (IntPtr* pSession = &session)
			{
				var err = UHR_CreateHTTPSession(pSession);
				if (err != Error.Ok)
				{
					throw new UHRException("Failed to create HttpSession: ", err);
				}
			}
		}

		~HttpSession() {}

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
					var err = UHR_Update(session, pResponses, (uint)responsesBuffer.Length, &count);
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
						err = UHR_DestroyRequests(session, toDelete, count);
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
				if (session != IntPtr.Zero)
				{
					var err = UHR_DestroyHTTPSession(session);
					if (err != Error.Ok)
					{
						throw new UHRException("Failed to destroy HttpSession: ", err);
					}
					session = IntPtr.Zero;
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
						session,
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

		private unsafe static void LogMessage(StringRef msg, IntPtr userData)
		{
			#if UHR_TESTS
			TestContext.Out.WriteLine(msg.ToStringAlloc());
			#else
			Debug.WriteLine(msg.ToStringAlloc());
			#endif
		}

		#region NativeBindings

		[DllImport(NativeLibrary.Name, CallingConvention = CallingConvention.Cdecl)]
		extern static void UHR_SetLoggingCallback(
			[MarshalAs(UnmanagedType.FunctionPtr)] LoggingCallback callback,
			IntPtr userData
		);

		[DllImport(NativeLibrary.Name, CallingConvention = CallingConvention.Cdecl)]
		extern static Error UHR_CreateHTTPSession(IntPtr* httpSessionHandleOut);

		[DllImport(NativeLibrary.Name, CallingConvention = CallingConvention.Cdecl)]
		extern static Error UHR_DestroyHTTPSession(IntPtr httpSessionHandle);

		[DllImport(NativeLibrary.Name, CallingConvention = CallingConvention.Cdecl)]
		extern static Error UHR_CreateRequest(
			IntPtr httpSessionHandle,
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
			IntPtr httpSessionHandle,
			Response* responsesOut,
			uint responsesCapacity,
			uint* responseCountOut
		);

		[DllImport(NativeLibrary.Name, CallingConvention = CallingConvention.Cdecl)]
		extern static Error UHR_DestroyRequests(
			IntPtr httpSessionHandle,
			uint* requestIDs,
			uint requestIDsCount
		);

		#endregion
	}

}
