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

#define UHR_ERR_OK 0
#define UHR_ERR_INVALID_CONTEXT 1
#define UHR_ERR_MISSING_REQUIRED_PARAMETER 2
#define UHR_ERR_INVALID_HTTP_METHOD 3
#define UHR_ERR_FAILED_TO_CREATE_REQUEST 4
#define UHR_ERR_UNKNOWN_ERROR_CODE 5
#define UHR_ERR_FAILED_TO_CREATE_CONTEXT 6
#define UHR_ERR_STRING_DECODING_ERROR 7

#define UHR_METHOD_GET 0
#define UHR_METHOD_HEAD 1
#define UHR_METHOD_POST 2
#define UHR_METHOD_PUT 3
#define UHR_METHOD_PATCH 4
#define UHR_METHOD_DELETE 5
#define UHR_METHOD_CONNECT 6
#define UHR_METHOD_OPTIONS 7
#define UHR_METHOD_TRACE 8

typedef uint32_t UHR_Error;
typedef uint32_t UHR_Method;
typedef uintptr_t UHR_HttpContext;
typedef uint32_t UHR_RequestId;

typedef struct {
    const uint16_t *characters;
    uint32_t length;
    uint32_t pad;
} UHR_StringRef;

typedef struct {
    UHR_StringRef name;
    UHR_StringRef value;
} UHR_Header;

typedef struct {
    const UHR_Header *headers;
    uint32_t count;
    uint32_t pad;
} UHR_HeadersData;

typedef struct {
    const char *body;
    uint32_t length;
    uint32_t pad;
} UHR_BodyData;

typedef struct {
    UHR_RequestId request_id;
    // In the case of a network error, http_status will be 0.
    // In the case of a failure to read the response body, http_status will be 0.
    // In the case of an HTTP error, http_status will reflect the HTTP status code.
    uint32_t http_status;
    UHR_HeadersData headers;
    UHR_BodyData body;
} UHR_Response;


// UHR_ErrorToString gets a message describing the provided error
UHR_API UHR_Error UHR_ErrorToString(
    UHR_Error err,
    UHR_StringRef* errorMessageOut
);

// UHR_CreateHTTPContext creates an HttpContext.
// A owns all the resources, and manages the http request requests.
// Returns:  A valid handle, or UHRHTTP_CONTEXT_INVALID if an error occurred.
//   Use UHRGetLastError to get the error message.
UHR_API UHR_Error UHR_CreateHTTPContext(
    UHR_HttpContext* httpContextHandleOut
);

// UHR_DestroyHTTPContext frees an HttpContext, and any requests that it is managing.
UHR_API UHR_Error UHR_DestroyHTTPContext(
    UHR_HttpContext httpContextHandle
);

// UHR_CreateRequest starts a new request.
// Returns:  The request id of the new request, or UHR_REQUEST_ID_INVALID if an error occurred.
//   Use UHRGetLastError to get the error message.
UHR_API UHR_Error UHR_CreateRequest(
    UHR_HttpContext httpContextHandle,
    UHR_StringRef url,
    UHR_Method method,
    UHR_Header* headers,
    uint32_t headersCount,
    char* body,
    uint32_t bodyLength,
    UHR_RequestId* ridOut
);

// UHR_Update polls for finished requests.
// responseCountOut receives the number of responses written to the output array.
UHR_API UHR_Error UHR_Update(
    UHR_HttpContext httpContextHandle,
    UHR_Response* responsesOut,
    uint32_t responsesCapacity,
    uint32_t* responseCountOut
);

// UHR_DestroyRequests frees memory associated with the specified request ids.
// Every request that is created must eventually be destroyed.
// However, destroying the httpContext is sufficient to also destroy all
// the requests it manages.
UHR_API UHR_Error UHR_DestroyRequests(
    UHR_HttpContext httpContextHandle,
    UHR_RequestId* requestIDs,
    uint32_t requestIDsCount
);

#ifdef __cplusplus
}
#endif
