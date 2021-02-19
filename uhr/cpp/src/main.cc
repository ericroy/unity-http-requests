#include <string>
#include <mutex>
#include <sstream>
#include <functional>
#include <unordered_map>

// For example program
#include <iostream>
#include <thread>
#include <chrono>

#define clue_LOG_EXPRESSION( severity, expr ) \
	g_log_sink.Acquire() << "UHR[" << clue::to_severity_text(severity) << "]: " << expr
#include <clue.hpp>
#include <curl/curl.h>

#include "UnityHttpRequests.h"
#include "init.h"
#include "util.h"
#include "log_sink.h"
#include "request.h"
#include "request_builder.h"
#include "session.h"

namespace {

const std::unordered_map<UHR_Error, const std::u16string> g_error_strings = {
	{ UHR_ERR_OK,                         uhr::ToUTF16("Ok") },
	{ UHR_ERR_INVALID_SESSION,            uhr::ToUTF16("The session handle was invalid") },
	{ UHR_ERR_MISSING_REQUIRED_PARAMETER, uhr::ToUTF16("A required function parameter was missing or null") },
	{ UHR_ERR_INVALID_HTTP_METHOD,        uhr::ToUTF16("Invalid HTTP method") },
	{ UHR_ERR_FAILED_TO_CREATE_REQUEST,   uhr::ToUTF16("Failed to create request") },
	{ UHR_ERR_UNKNOWN_ERROR_CODE,         uhr::ToUTF16("Unknown error code") },
	{ UHR_ERR_FAILED_TO_CREATE_SESSION,   uhr::ToUTF16("Failed to create session") },
	{ UHR_ERR_FAILED_TO_UPDATE_SESSION,   uhr::ToUTF16("Failed to update session") },
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

// Not inside anonymous namespace, so it can be accessed by the clue
// logging macros in other code units.
uhr::LogSink g_log_sink;

UHR_API void UHR_SetLoggingCallback(UHR_LoggingCallback callback, void* user_data) {
	g_log_sink.Set(callback, user_data);
}

UHR_API UHR_Error UHR_ErrorToString(UHR_Error err, UHR_StringRef* error_message_out) {
	if (error_message_out == nullptr)
		return UHR_ERR_MISSING_REQUIRED_PARAMETER;

	auto iter = g_error_strings.find(err);
	if (iter == end(g_error_strings))
		return UHR_ERR_UNKNOWN_ERROR_CODE;

	error_message_out->characters = reinterpret_cast<const uint16_t *>(iter->second.c_str());
	error_message_out->length = static_cast<uint32_t>(iter->second.size());
	return UHR_ERR_OK;
}

UHR_API UHR_Error UHR_CreateHTTPSession(UHR_HttpSession* http_session_handle_out) {
	if (http_session_handle_out == nullptr)
		return UHR_ERR_MISSING_REQUIRED_PARAMETER;
	
	// If we leave this function without returning a valid
	// session, then we must deinit to balance.  Otherwise,
	// the deinit will be called when the caller destroys their session handle.
	uhr::Init();

	auto session = uhr::Session::Create();
	if (session == nullptr) {
		uhr::Deinit();
		return UHR_ERR_FAILED_TO_CREATE_SESSION;
	}

	*http_session_handle_out = reinterpret_cast<UHR_HttpSession>(session.release());
	return UHR_ERR_OK;
}

UHR_API UHR_Error UHR_DestroyHTTPSession(UHR_HttpSession http_session_handle) {
	if (http_session_handle == 0)
		return UHR_ERR_INVALID_SESSION;
	
	auto *session = reinterpret_cast<uhr::Session *>(http_session_handle);
	delete session;
	uhr::Deinit();
	return UHR_ERR_OK;
}

UHR_API UHR_Error UHR_CreateRequest(
	UHR_HttpSession http_session_handle,
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

	if (http_session_handle == 0)
		return UHR_ERR_INVALID_SESSION;

	auto method_iter = g_method_strings.find(method);
	if (method_iter == std::end(g_method_strings))
		return UHR_ERR_INVALID_HTTP_METHOD;

	auto *session = reinterpret_cast<uhr::Session *>(http_session_handle);
	
	auto rid = session->GetNextRid();
	
	auto builder = uhr::RequestBuilder::Create(rid, session->ca_certs());
	if (!builder)
		return UHR_ERR_FAILED_TO_CREATE_REQUEST;
	
	builder->SetMethod(method_iter->second);
	
	builder->SetUrl(uhr::ToUTF8(url));

	if (body_length > 0)
		builder->SetBody(body, body_length);
	
	for (auto i = 0u; i < headers_count; ++i)
		builder->AddHeader(uhr::ToUTF8(headers[i].name), uhr::ToUTF8(headers[i].value));
	
	auto req = builder->Build();
	if (!req)
		return UHR_ERR_FAILED_TO_CREATE_REQUEST;

	if (!session->StartRequest(std::move(req)))
		return UHR_ERR_FAILED_TO_CREATE_REQUEST;
	
	*rid_out = rid;
	return UHR_ERR_OK;
}

UHR_API UHR_Error UHR_Update(UHR_HttpSession http_session_handle, UHR_Response* responses_out, uint32_t responses_capacity, uint32_t* response_count_out) {
	if (response_count_out == nullptr)
		return UHR_ERR_MISSING_REQUIRED_PARAMETER;

	if (http_session_handle == 0)
		return UHR_ERR_INVALID_SESSION;

	auto *session = reinterpret_cast<uhr::Session *>(http_session_handle);
	if (!session->Update())
		return UHR_ERR_FAILED_TO_UPDATE_SESSION;

	uint32_t count = 0;
	while (count < responses_capacity && session->GetNextNoncancelledResponse(&responses_out[count]))
		++count;
	
	*response_count_out = count;
	return UHR_ERR_OK;
}

UHR_API UHR_Error UHR_DestroyRequests(
	UHR_HttpSession http_session_handle,
	UHR_RequestId* request_ids,
	uint32_t request_ids_count
) {
	if (request_ids_count > 0 && request_ids == nullptr)
		return UHR_ERR_MISSING_REQUIRED_PARAMETER;

	if (http_session_handle == 0)
		return UHR_ERR_INVALID_SESSION;

	auto *session = reinterpret_cast<uhr::Session *>(http_session_handle);
	for (auto i = 0u; i < request_ids_count; ++i)
		session->DestroyRequest(request_ids[0]);

	return UHR_ERR_OK;
}

int main(void) {
	UHR_HttpSession session = 0;
	UHR_RequestId rid = 0;
	UHR_Error err = UHR_ERR_OK;
	auto url = uhr::ToUTF16("https://example.com");
	UHR_Response responses[8] = {};
	std::uint32_t responses_ready = 0;
	UHR_RequestId request_ids[8] = {};
	auto log_callback = [](UHR_StringRef msg, void *user_data) {
		std::cout << uhr::ToUTF8(msg) << std::endl;
	};

	UHR_SetLoggingCallback(log_callback, nullptr);

	err = UHR_CreateHTTPSession(&session);
	if (err != UHR_ERR_OK) {
		UHR_StringRef sr = {};
		UHR_ErrorToString(err, &sr);
		LOG_ERROR("UHR_CreateHTTPSession: " << uhr::ToUTF8(sr));
		goto done;
	}

	LOG_INFO("Created session");

	err = UHR_CreateRequest(session, uhr::ToStringRef(url), UHR_METHOD_GET,
		nullptr,	// headers
		0,			// headers_count
		nullptr,	// body
		0,			// body_length
		&rid
	);
	if (err != UHR_ERR_OK) {
		UHR_StringRef sr = {};
		UHR_ErrorToString(err, &sr);
		LOG_ERROR("UHR_CreateRequest: " << uhr::ToUTF8(sr));
		goto done;
	}

	LOG_INFO("Request created");

	while (true) {
		err = UHR_Update(session, responses, sizeof(responses) / sizeof(responses[0]), &responses_ready);
		if (err != UHR_ERR_OK) {
			UHR_StringRef sr = {};
			UHR_ErrorToString(err, &sr);
			LOG_ERROR("UHR_Update: " << uhr::ToUTF8(sr));
			goto done;
		}

		if (responses_ready > 0) {
			auto &response = responses[0];
			LOG_DEBUG(std::string(response.body.body, response.body.length));

			request_ids[0] = rid;
			err = UHR_DestroyRequests(session, request_ids, 1);
			if (err != UHR_ERR_OK) {
				UHR_StringRef sr = {};
				UHR_ErrorToString(err, &sr);
				LOG_ERROR("UHR_DestroyRequests: " << uhr::ToUTF8(sr));
				goto done;
			}
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}


done:
	UHR_DestroyHTTPSession(session);
	LOG_INFO("Destroyed session");
	return 0;
}