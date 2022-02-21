using System;
using System.Collections.Generic;
using UnityHttpRequests;

public class RequestDispatcher : MonoBehaviour
{
	private static readonly Dictionary<string, string> requestHeaders = new Dictionary<string, string>{
		{ "User-Agent", "FooAgent" },
		{ "Accept-Encoding", "gzip" },
		{ "Accept", "application/json" },
		{ "Content-Type", "application/json" },
	};
	
	private HttpSession session = new HttpSession();
	private byte[] body = new byte[4096];
	private Dictionary<int, HttpSession.RequestCompleteDelegate> inFlight = new Dictionary<int, HttpSession.RequestCompleteDelegate>(16);

	// Note: For this example to be allocation free, the caller of Get() or Post() needs to
	// cache their HttpSession.RequestCompleteDelegate in a member variable so they are not
	// creating a new delegate instance for every call.
	public void Get(string url, HttpSession.RequestCompleteDelegate onResponse)
	{
		int rid = session.Get(url, requestHeaders);
		inFlight[rid] = onResponse;
	}

	// See the comment on the Get() method.
	public void Post(string url, byte[] requestBody, int requestBodyLength, HttpSession.RequestCompleteDelegate onResponse)
	{
		int rid = session.Post(url, body, bodyLen, requestHeaders);
		inFlight[rid] = onResponse;
	}

	private void OnEnable()
	{
		session.RequestComplete += OnResponse;
	}

	private void OnDisable()
	{
		session.RequestComplete -= OnResponse;
	}

	private void Update()
	{
		session.Update();
	}

	private void OnDestroy()
	{
		session.Dispose();
		session = null;
	}

	private void OnResponse(ref Response res)
	{
		// Lookup the response handler for this request id
		HttpSession.RequestCompleteDelegate onResponse = null;
		if (inFlight.TryGet(res.RequestId, onResponse))
		{
			// Deliver the response
			onResponse?.Invoke(ref res);
			inFlight.Remove(res.RequestId);
		}
	}
}