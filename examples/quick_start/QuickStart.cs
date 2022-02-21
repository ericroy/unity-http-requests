using System;
using System.Collections.Generic;
using UnityHttpRequests;
using Cysharp.Text;   // Using ZString for this example, but you don't have to.

public class QuickStart : MonoBehaviour
{
	// In most cases, the set of request headers will be static, so we
	// allocate them up front and reuse the dictionary.
	private static readonly Dictionary<string, string> requestHeaders = new Dictionary<string, string>{
		{ "User-Agent", "FooAgent" },
		{ "Accept-Encoding", "gzip" },
		{ "Accept", "application/json" },
		{ "Content-Type", "application/json" },
	};

	private HttpSession session = new HttpSession();
	
	private byte[] buffer = new byte[4096];

	// We'll use this class from the ZString library as an example serialization method
	private Utf8ValueStringBuilder stringBuilder = ZString.CreateUtf8StringBuilder();
	
	private int lastRid = 0;

	// Sends a nonsense post request.
	public void SendRequest()
	{
		// How you serialize data into your scratch buffer is up to you, and is not managed by the library.
		// Some low/no allocation options (depending on how you use them):
		// https://github.com/Cysharp/ZString
		// https://github.com/neuecc/Utf8Json
		// https://github.com/neuecc/MessagePack-CSharp
		int bodyLen = 0;
		stringBuilder.Clear();
		stringBuilder.AppendFormat("{\"foo\": \"{0}\", \"baz\": {1}}", "bar", 33);
		stringBuilder.TryCopyTo(buffer, out bodyLen);
			
		// The `rid` (Request ID) returned by from the Post() method is a unique request identifier.
		// Incoming responses will contain the rid so you can associate them with the request, if you want.
		lastRid = session.Post("https://httpbin.org/post", buffer, bodyLen, requestHeaders);
	}

	private void OnEnable()
	{
		// Start listening for responses
		session.RequestComplete += OnResponse;
	}

	private void OnDisable()
	{
		// Stop listening for responses
		session.RequestComplete -= OnResponse;
	}

	private void Update()
	{
		// You must tick the session each frame,
		// otherwise you will not receive response events.
		session.Update();
	}

	private void OnDestroy()
	{
		// The HttpSession is IDisposable!
		session.Dispose();
		session = null;
	}

	// The application must not hold references to anything in the Response object
	// beyond the scope of this function, so if we want to keep the response body, for
	// example, it should be copied somewhere permanent.
	private void OnResponse(ref Response res)
	{
		// For the sake of this example, we'll ignore any response unless it corresponds
		// to our most recent request.  See examples/response_dispatcher for a class that
		// keeps a response handler callback per-request, so the responses can be delivered
		// directly to the party that initiated the request.
		if (res.RequestId != lastRid)
		{
			// Not the response for the most recent request
			return;
		}

		if (res.HttpStatus == 0)
		{
			// Network error
			return;
		}

		if (res.HttpStatus < 200 || res.HttpStatus >= 300)
		{
			// Http error
			return;
		}

		// If the content type header starts with "application/json"
		if (res.Headers.HeaderStartsWith("Content-Type", "application/json", false))
		{
			// For the purpose of giving useful errors, we may want to actually get the header value.
			// This involves allocating a string.  Any api function that allocates has the suffix "Alloc".
			Debug.LogError($"Response was not json: ${res.Headers.GetHeaderAlloc("Content-Type")}");
			return;
		}
		
		// Copy the response body into our scratch buffer
		int copiedBytes = res.Body.CopyTo(buffer);
		if ((uint)copiedBytes < res.Body.Length)
		{
			// Truncated!
			return;
		}

		// Do something with the response body, parse it perhaps.
		// ...
	}
}