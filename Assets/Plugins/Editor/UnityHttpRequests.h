/* Code generated by cmd/cgo; DO NOT EDIT. */

/* package github.com/ericroy/unity-http-requests/Src~/go/cmd/unity_http_requests */


#line 1 "cgo-builtin-export-prolog"

#include <stddef.h> /* for ptrdiff_t below */

#ifndef GO_CGO_EXPORT_PROLOGUE_H
#define GO_CGO_EXPORT_PROLOGUE_H

#ifndef GO_CGO_GOSTRING_TYPEDEF
typedef struct { const char *p; ptrdiff_t n; } _GoString_;
#endif

#endif

/* Start of preamble from import "C" comments.  */


#line 3 "main.go"
 #include <stdint.h>
 #define UHR_REQUEST_ID_INVALID 0
 #define UHR_HTTP_CONTEXT_INVALID 0

 #define UHR_METHOD_GET 0
 #define UHR_METHOD_POST 1

 typedef uintptr_t UHR_HttpContext;
 typedef int32_t UHR_RequestId;

 typedef struct {
 	const uint16_t *characters;
 	int32_t length;
 } UHR_StringRef;

 typedef struct {
 	UHR_StringRef name;
 	UHR_StringRef value;
 } UHR_Header;

 typedef struct {
 	UHR_RequestId request_id;
 	int32_t http_status;
 	UHR_Header *response_headers;
 	int32_t response_headers_count;
 	const char *response_body;
 	int32_t response_body_length;
 } UHR_Response;

#line 1 "cgo-generated-wrapper"


/* End of preamble from import "C" comments.  */


/* Start of boilerplate cgo prologue.  */
#line 1 "cgo-gcc-export-header-prolog"

#ifndef GO_CGO_PROLOGUE_H
#define GO_CGO_PROLOGUE_H

typedef signed char GoInt8;
typedef unsigned char GoUint8;
typedef short GoInt16;
typedef unsigned short GoUint16;
typedef int GoInt32;
typedef unsigned int GoUint32;
typedef long long GoInt64;
typedef unsigned long long GoUint64;
typedef GoInt64 GoInt;
typedef GoUint64 GoUint;
typedef __SIZE_TYPE__ GoUintptr;
typedef float GoFloat32;
typedef double GoFloat64;
typedef float _Complex GoComplex64;
typedef double _Complex GoComplex128;

/*
  static assertion to make sure the file is being used on architecture
  at least with matching size of GoInt.
*/
typedef char _check_for_64_bit_pointer_matching_GoInt[sizeof(void*)==64/8 ? 1:-1];

#ifndef GO_CGO_GOSTRING_TYPEDEF
typedef _GoString_ GoString;
#endif
typedef void *GoMap;
typedef void *GoChan;
typedef struct { void *t; void *v; } GoInterface;
typedef struct { void *data; GoInt len; GoInt cap; } GoSlice;

#endif

/* End of boilerplate cgo prologue.  */

#ifdef __cplusplus
extern "C" {
#endif


// UHR_GetLastError gets a message describing the latest error
extern void UHR_GetLastError(UHR_StringRef* errorOut);

// UHR_CreateHTTPContext creates an HttpContext.
// A owns all the resources, and manages the http request requests.
// Returns:  A valid handle, or UHRHTTP_CONTEXT_INVALID if an error occurred.
//   Use UHRGetLastError to get the error message.
extern UHR_HttpContext UHR_CreateHTTPContext();

// UHR_DestroyHTTPContext frees an HttpContext, and any requests that it is managing.
extern void UHR_DestroyHTTPContext(UHR_HttpContext httpContextHandle);

// UHR_CreateRequest starts a new request.
// Returns:  The request id of the new request, or UHR_REQUEST_ID_INVALID if an error occurred.
//   Use UHRGetLastError to get the error message.
extern UHR_RequestId UHR_CreateRequest(UHR_HttpContext httpContextHandle, UHR_StringRef url, int32_t method, UHR_Header* headers, int32_t headersCount, char* body, int32_t bodyLength);

// UHR_Update polls for finished requests.
// Returns: number of responses written to output array, or -1 in the case that the entire operation failed.
//   Use UHRGetLastError to get the error message.
//   Individual requests may also fail:
//      - In the case of a network error, http_status for that response will be -1.
//      - In the case of a failure to read the response body, http_status for that response will be -1.
//		- In the case of an HTTP error, http_status for that response will reflect the HTTP error code.
extern int32_t UHR_Update(UHR_HttpContext httpContextHandle, UHR_Response* responsesOut, int32_t responsesCapacity);

// UHR_DestroyRequests frees memory associated with the specified request ids.
// Every request that is created must eventually be destroyed.
// However, destroying the httpContext is sufficient to also destroy all
// the requests it manages.
//
// Returns: Zero on success, or -1 on failure.
//   Use UHRGetLastError to get the error message.
extern int32_t UHR_DestroyRequests(UHR_HttpContext httpContextHandle, UHR_RequestId* requestIDs, int32_t requestIDsCount);

#ifdef __cplusplus
}
#endif
