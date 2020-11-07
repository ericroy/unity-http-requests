#include "http_requests.h"
#include "http_context.h"
#include <Windows.h>

namespace {
    std::u16string g_last_error;
}

void UHR_GetLastError(UHR_StringRef *error_out) {
    if (error_out == nullptr)
        return;

    error_out->characters = reinterpret_cast<const short*>(g_last_error.c_str());
    error_out->length = g_last_error.length();
}

UHR_HttpContext UHR_CreateHttpContext() {
    auto* context = new http_requests::HttpContext();
    return static_cast<UHR_HttpContext>(context);
}

void UHR_DestroyHttpContext(UHR_HttpContext handle) {
    auto* context = static_cast<http_requests::HttpContext *>(handle);
    if (context != nullptr)
        delete context;
}

UHR_RequestId UHR_CreateRequest(
	UHR_HttpContext handle,
	UHR_StringRef url,
	int method,
	UHR_Header *request_headers,
	int request_headers_count,
	const char *request_body,
	int request_body_length
) {
    auto* context = static_cast<http_requests::HttpContext *>(handle);
    if (context == nullptr) {
        g_last_error = u"Invalid HttpContext handle";
        return UHR_HTTP_CONTEXT_INVALID;
    }
    return context->CreateRequest(url, method, request_headers, request_headers_count, request_body, request_body_length, &g_last_error);
}

int UHR_Update(UHR_HttpContext handle, UHR_Response* responses_out, int responses_capacity) {
    auto* context = static_cast<http_requests::HttpContext *>(handle);
    if (context == nullptr) {
        g_last_error = u"Invalid HttpContext handle";
        return -1;
    }
    return context->Update(responses_out, responses_capacity, &g_last_error);
}

int UHR_DestroyRequests(UHR_HttpContext handle, UHR_RequestId* request_ids, int request_ids_count) {
    auto* context = static_cast<http_requests::HttpContext *>(handle);
    if (context == nullptr) {
        g_last_error = u"Invalid HttpContext handle";
        return -1;
    }
    return context->DestroyRequests(request_ids, request_ids_count, &g_last_error);
}

BOOL WINAPI DllMain(HINSTANCE hinst_dll, DWORD reason, LPVOID reserved) {
    return TRUE;
}