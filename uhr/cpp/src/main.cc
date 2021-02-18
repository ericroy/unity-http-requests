#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <unordered_map>
#include <curl/curl.h>
#include "UnityHttpRequests.h"
#include "init.h"
#include "util.h"
#include "request.h"
#include "context.h"

namespace {

const std::unordered_map<UHR_Error, const std::u16string> g_error_strings = {
	{ UHR_ERR_OK,                         uhr::ToUTF16("Ok") },
	{ UHR_ERR_INVALID_CONTEXT,            uhr::ToUTF16("The context handle was invalid") },
	{ UHR_ERR_MISSING_REQUIRED_PARAMETER, uhr::ToUTF16("A required function parameter was missing or null") },
	{ UHR_ERR_INVALID_HTTP_METHOD,        uhr::ToUTF16("Invalid HTTP method") },
	{ UHR_ERR_FAILED_TO_CREATE_REQUEST,   uhr::ToUTF16("Failed to create request") },
	{ UHR_ERR_UNKNOWN_ERROR_CODE,         uhr::ToUTF16("Unknown error code") },
	{ UHR_ERR_FAILED_TO_CREATE_CONTEXT,   uhr::ToUTF16("Failed to create context") },
	{ UHR_ERR_FAILED_TO_DESTROY_CONTEXT,  uhr::ToUTF16("Failed to destroy context") },
	{ UHR_ERR_FAILED_TO_UPDATE_CONTEXT,   uhr::ToUTF16("Failed to update context") },
};

const std::unordered_map<UHR_Method, const std::string> g_method_strings = {
	{ UHR_METHOD_GET,     "GET" },
	{ UHR_METHOD_HEAD,    "HEAD" },
	{ UHR_METHOD_POST,    "POST" },
	{ UHR_METHOD_PUT,     "PUT" },
	{ UHR_METHOD_PATCH,   "PATCH" },
	{ UHR_METHOD_DELETE,  "DELETE" },
	{ UHR_METHOD_CONNECT, "CONNECT" },
	{ UHR_METHOD_OPTIONS, "OPTIONS" },
	{ UHR_METHOD_TRACE,   "TRACE" },
};

} // namespace


UHR_API UHR_Error UHR_ErrorToString(
    UHR_Error err,
    UHR_StringRef* error_message_out
) {
	if (error_message_out == nullptr)
		return UHR_ERR_MISSING_REQUIRED_PARAMETER;

	auto iter = g_error_strings.find(err);
	if (iter == end(g_error_strings))
		return UHR_ERR_UNKNOWN_ERROR_CODE;

	error_message_out->characters = reinterpret_cast<const uint16_t *>(iter->second.c_str());
	error_message_out->length = static_cast<uint32_t>(iter->second.size());
	return UHR_ERR_OK;
}

UHR_API UHR_Error UHR_CreateHTTPContext(
    UHR_HttpContext* http_context_handle_out
) {
	if (http_context_handle_out == nullptr)
		return UHR_ERR_MISSING_REQUIRED_PARAMETER;
	
	// If we leave this function without returning a valid
	// context, then we must deinit to balance.  Otherwise,
	// the deinit will be called when they destroy their handle.
	uhr::Init();

	auto context = uhr::Context::Create();
	if (context == nullptr) {
		uhr::Deinit();
		return UHR_ERR_FAILED_TO_CREATE_CONTEXT;
	}

	*http_context_handle_out = reinterpret_cast<UHR_HttpContext>(context.release());
	return UHR_ERR_OK;
}

UHR_API UHR_Error UHR_DestroyHTTPContext(
    UHR_HttpContext http_context_handle
) {
	if (http_context_handle == 0)
		return UHR_ERR_INVALID_CONTEXT;
	
	auto *context = reinterpret_cast<uhr::Context *>(http_context_handle);
	delete context;
	uhr::Deinit();
	return UHR_ERR_OK;
}

UHR_API UHR_Error UHR_CreateRequest(
    UHR_HttpContext http_context_handle,
    UHR_StringRef url,
    UHR_Method method,
    UHR_Header* headers,
    uint32_t headers_count,
    char* body,
    uint32_t body_length,
    UHR_RequestId* rid_out
) {
	if (rid_out == nullptr)
		return UHR_ERR_MISSING_REQUIRED_PARAMETER;

	if (headers_count > 0 && headers == nullptr)
		return UHR_ERR_MISSING_REQUIRED_PARAMETER;

	if (body_length > 0 && body == nullptr)
		return UHR_ERR_MISSING_REQUIRED_PARAMETER;

	if (http_context_handle == 0)
		return UHR_ERR_INVALID_CONTEXT;

	auto method_iter = g_method_strings.find(method);
	if (method_iter == std::end(g_method_strings))
		return UHR_ERR_INVALID_HTTP_METHOD;

	auto *context = reinterpret_cast<uhr::Context *>(http_context_handle);
	auto req = context->CreateRequest();
	if (req == nullptr)
		return UHR_ERR_FAILED_TO_CREATE_REQUEST;
	
	if (!req->SetMethod(method_iter->second))
		return UHR_ERR_FAILED_TO_CREATE_REQUEST;

	if (!req->SetUrl(uhr::ToUTF8(url)))
		return UHR_ERR_FAILED_TO_CREATE_REQUEST;

	if (body_length > 0 && !req->SetBody(body, body_length))
		return UHR_ERR_FAILED_TO_CREATE_REQUEST;

	if (headers_count > 0) {
		for (auto i = 0u; i < headers_count; ++i) {
			if (!req->AddHeader(uhr::ToUTF8(headers[i].name), uhr::ToUTF8(headers[i].value)))
				return UHR_ERR_FAILED_TO_CREATE_REQUEST;
		}
	}
	
	auto rid = req->rid();
	if (!context->StartRequest(move(req)))
		return UHR_ERR_FAILED_TO_CREATE_REQUEST;
	
	*rid_out = rid;
	return UHR_ERR_OK;
}

UHR_API UHR_Error UHR_Update(
    UHR_HttpContext http_context_handle,
    UHR_Response* responses_out,
    uint32_t responses_capacity,
    uint32_t* response_count_out
) {
	if (response_count_out == nullptr)
		return UHR_ERR_MISSING_REQUIRED_PARAMETER;

	if (http_context_handle == 0)
		return UHR_ERR_INVALID_CONTEXT;

	auto *context = reinterpret_cast<uhr::Context *>(http_context_handle);

	if (!context->Update())
		return UHR_ERR_FAILED_TO_UPDATE_CONTEXT;

	uint32_t count = 0;
	uhr::Request *req = nullptr;
	while (count < responses_capacity && (req = context->GetNextCompleted()) != nullptr) {
		// Only return responses to the caller for non-cancelled requests.
		if (!req->cancelled()) {
			req->GetResponse(&responses_out[count]);
			++count;
		}
	}
	
	*response_count_out = count;
	return UHR_ERR_OK;
}

UHR_API UHR_Error UHR_DestroyRequests(
    UHR_HttpContext http_context_handle,
    UHR_RequestId* request_ids,
    uint32_t request_ids_count
) {
	if (request_ids_count > 0 && request_ids == nullptr) {
		return UHR_ERR_MISSING_REQUIRED_PARAMETER;
	}

	if (http_context_handle == 0) {
		return UHR_ERR_INVALID_CONTEXT;
	}

	auto *context = reinterpret_cast<uhr::Context *>(http_context_handle);

	for (auto i = 0u; i < request_ids_count; ++i) {
		context->DestroyRequest(request_ids[0]);
	}

	return UHR_ERR_OK;
}

UHR_StringRef to_ref(const std::u16string &s) {
	UHR_StringRef sr = {};
	sr.characters = reinterpret_cast<const uint16_t *>(s.c_str());
	sr.length = s.size();
	return sr;
}

int main(void) {
	UHR_HttpContext context = 0;
	UHR_RequestId rid = 0;
	UHR_Error err = UHR_ERR_OK;
	auto url = uhr::ToUTF16("https://example.com");
	UHR_Response responses[8] = {};
	std::uint32_t responses_ready = 0;
	UHR_RequestId request_ids[8] = {};
	
	err = UHR_CreateHTTPContext(&context);
	if (err != UHR_ERR_OK) {
		UHR_StringRef sr = {};
		UHR_ErrorToString(err, &sr);
		std::cout << uhr::ToUTF8(sr) << std::endl;
		goto done;
	}

	std::cout << "Created context" << std::endl;

	err = UHR_CreateRequest(context, to_ref(url), UHR_METHOD_GET,
		nullptr,	// headers
		0,			// headers_count
		nullptr,	// body
		0,			// body_length
		&rid
	);
	if (err != UHR_ERR_OK) {
		UHR_StringRef sr = {};
		UHR_ErrorToString(err, &sr);
		std::cout << uhr::ToUTF8(sr) << std::endl;
		goto done;
	}

	std::cout << "Request created" << std::endl;

	while (true) {
		err = UHR_Update(context, responses, sizeof(responses) / sizeof(responses[0]), &responses_ready);
		if (err != UHR_ERR_OK) {
			UHR_StringRef sr = {};
			UHR_ErrorToString(err, &sr);
			std::cout << uhr::ToUTF8(sr) << std::endl;
			goto done;
		}

		if (responses_ready > 0) {
			auto &response = responses[0];
			std::cout << "BODY:" << std::endl;
			std::cout << std::string(response.body.body, response.body.length) << std::endl;

			request_ids[0] = rid;
			err = UHR_DestroyRequests(context, request_ids, 1);
			if (err != UHR_ERR_OK) {
				UHR_StringRef sr = {};
				UHR_ErrorToString(err, &sr);
				std::cout << uhr::ToUTF8(sr) << std::endl;
				goto done;
			}
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}


done:
	UHR_DestroyHTTPContext(context);
	std::cout << "Destroyed context" << std::endl;
	return 0;
}