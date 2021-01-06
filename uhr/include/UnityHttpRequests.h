#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// UHR_PLATFORM_* defines
#if defined(_WIN32) || defined(_WIN64)
    #define UHR_PLATFORM_WIN
#elif defined(__APPLE__)
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR
        #define UHR_PLATFORM_IOS
    #elif TARGET_OS_IPHONE
        #define UHR_PLATFORM_IOS
    #elif TARGET_OS_MAC
        #define UHR_PLATFORM_MAC
    #else
        #error "Platform not supported"
    #endif
#elif defined(__linux__)
    #if defined(__ANDROID__)
        #define UHR_PLATFORM_ANDROID
    #else
        #define UHR_PLATFORM_LINUX
    #endif
#else
    #error "Platform not supported"
#endif

// UHR_API define
#if defined(UHR_CGO)
    #define UHR_API extern
#else
    #ifdef UHR_PLATFORM_WIN
        #ifdef UHR_BUILDING_SHARED
            #define UHR_API __declspec(dllexport)
        #else
            #define UHR_API __declspec(dllimport)
        #endif
    #else
        #define UHR_API __attribute__((visibility("default")))
    #endif
#endif

#define UHR_REQUEST_ID_INVALID 0
#define UHR_HTTP_CONTEXT_INVALID 0

#define UHR_METHOD_GET 0
#define UHR_METHOD_POST 1

typedef uintptr_t UHR_HttpContext;
typedef int32_t UHR_RequestId;

typedef struct {
    const uint16_t *characters;
    int32_t length;
    int32_t pad;
} UHR_StringRef;

typedef struct {
    UHR_StringRef name;
    UHR_StringRef value;
} UHR_Header;

typedef struct {
    const UHR_Header *headers;
    int32_t count;
    int32_t pad;
} UHR_HeadersData;

typedef struct {
    const char *body;
    int32_t length;
    int32_t pad;
} UHR_BodyData;

typedef struct {
    UHR_RequestId request_id;
    int32_t http_status;
    UHR_HeadersData headers;
    UHR_BodyData body;
} UHR_Response;


// UHR_GetLastError gets a message describing the latest error
UHR_API void UHR_GetLastError(UHR_StringRef* errorOut);

// UHR_CreateHTTPContext creates an HttpContext.
// A owns all the resources, and manages the http request requests.
// Returns:  A valid handle, or UHRHTTP_CONTEXT_INVALID if an error occurred.
//   Use UHRGetLastError to get the error message.
UHR_API UHR_HttpContext UHR_CreateHTTPContext(void);

// UHR_DestroyHTTPContext frees an HttpContext, and any requests that it is managing.
UHR_API void UHR_DestroyHTTPContext(UHR_HttpContext httpContextHandle);

// UHR_CreateRequest starts a new request.
// Returns:  The request id of the new request, or UHR_REQUEST_ID_INVALID if an error occurred.
//   Use UHRGetLastError to get the error message.
UHR_API UHR_RequestId UHR_CreateRequest(UHR_HttpContext httpContextHandle, UHR_StringRef url, int32_t method, UHR_Header* headers, int32_t headersCount, char* body, int32_t bodyLength);

// UHR_Update polls for finished requests.
// Returns: number of responses written to output array, or -1 in the case that the entire operation failed.
//   Use UHRGetLastError to get the error message.
//   Individual requests may also fail:
//      - In the case of a network error, http_status for that response will be -1.
//      - In the case of a failure to read the response body, http_status for that response will be -1.
//		- In the case of an HTTP error, http_status for that response will reflect the HTTP error code.
UHR_API int32_t UHR_Update(UHR_HttpContext httpContextHandle, UHR_Response* responsesOut, int32_t responsesCapacity);

// UHR_DestroyRequests frees memory associated with the specified request ids.
// Every request that is created must eventually be destroyed.
// However, destroying the httpContext is sufficient to also destroy all
// the requests it manages.
//
// Returns: Zero on success, or -1 on failure.
//   Use UHRGetLastError to get the error message.
UHR_API int32_t UHR_DestroyRequests(UHR_HttpContext httpContextHandle, UHR_RequestId* requestIDs, int32_t requestIDsCount);

#ifdef __cplusplus
}
#endif
