#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef UHR_DLL_EXPORT
#define UHR_API __declspec(dllexport)
#else
#define UHR_API __declspec(dllimport)
#endif

#define UHR_REQUEST_ID_INVALID 0
#define UHR_HTTP_CONTEXT_INVALID 0

#define UHR_METHOD_GET 0
#define UHR_METHOD_POST 1

typedef void* UHR_HttpContext;
typedef int UHR_RequestId;

// UTF16 Strings (two-bytes per char)
struct UHR_StringRef {
	const short *characters;
	int length;
};

struct UHR_Header {
	UHR_StringRef name;
	UHR_StringRef value;
};

struct UHR_Response {
	UHR_RequestId request_id;
	int http_status;
	UHR_Header *response_headers;
	int response_headers_count;
	const char *response_body;
	int response_body_length;
};

// Gets a message describing the latest error
UHR_API void UHR_GetLastError(UHR_StringRef *error_out);

// Creates a context.
// A owns all the resources, and manages the http request requests.
// Returns:  A valid handle, or UHR_HTTP_CONTEXT_INVALID if an error occurred.
//   Use UHR_GetLastError to get the error message.
UHR_API UHR_HttpContext UHR_CreateHttpContext();

// Frees a context, and any requests that it is managing.
UHR_API void UHR_DestroyHttpContext(UHR_HttpContext context);

// Starts a new request.
// Returns:  The request id of the new request, or UHR_REQUEST_ID_INVALID if an error occurred.
//   Use UHR_GetLastError to get the error message.
UHR_API UHR_RequestId UHR_CreateRequest(
	UHR_HttpContext context,
	UHR_StringRef url,
	int method,
	const UHR_Header *request_headers,
	int request_headers_count,
	const char *request_body,
	int request_body_length
);

// Returns: number of responses written to output array, or -1 in the case of an error.
//   Use UHR_GetLastError to get the error message.
UHR_API int UHR_Update(UHR_HttpContext context, UHR_Response* responses_out, int responses_capacity);

// Frees memory associated with the specified request ids.
// Every request that is created must eventually be destroyed.
// However, destroying the context is sufficient to also destroy all
// the requests it manages.
//
// Returns: Number of requests destroyed, or -1 on complete failure.
//   If the number returned is less than request_ids_count, then some operations failed.
//   Use UHR_GetLastError to get the error message.
UHR_API int UHR_DestroyRequests(UHR_HttpContext context, UHR_RequestId* request_ids, int request_ids_count);

#ifdef __cplusplus
}
#endif