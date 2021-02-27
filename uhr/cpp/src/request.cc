#include "request.h"
#include "logging.h"
#include <cstring>
#include <sstream>
#include "util.h"

namespace uhr {

	Request::~Request() {
		Cleanup();
	}

	Request::Request(UHR_RequestId rid, CURL *easy) : rid_(rid), easy_(easy) {}

	bool Request::Attach(CURLM *multi) {
		auto res = curl_multi_add_handle(multi, easy_);
		if (res != CURLM_OK) {
			UHR_LOG_CRITICAL("Failed to attach Easy handle to Multi handle: " << curl_multi_strerror(res));
			return false;
		}
		return true;
	}

	void Request::Detach(CURLM *multi) {
		auto res = curl_multi_remove_handle(multi, easy_);
		if (res != CURLM_OK)
			UHR_LOG_CRITICAL("Failed to detach Easy handle from Multi handle: " << curl_multi_strerror(res));
	}

	void Request::OnComplete() {
		completed_ = true;

		// Get the status code
		long status = 0;
		http_status_ = 0;
		auto res = curl_easy_getinfo(easy_, CURLINFO_RESPONSE_CODE, &status);
		if (res == CURLE_OK) {
			http_status_ = static_cast<uint32_t>(status);
		} else {
			UHR_LOG_CRITICAL("Failed to get http status from curl: " << curl_easy_strerror(res));
		}
	}

	void Request::Cleanup() {
		if (easy_ != nullptr) {
			curl_easy_cleanup(easy_);
			easy_ = nullptr;
		}

		if (request_headers_list_ != nullptr) {
			curl_slist_free_all(request_headers_list_);
			request_headers_list_ = nullptr;
		}

		// Minimize memory footprint by clearing some of the request-related data.
		// Keep the response-related data around though.
		method_.clear();
		url_.clear();
		request_headers_.clear();
		request_body_.clear();
	}

	UHR_Response Request::GetResponse() {
		UHR_HeadersData hd = {};
		hd.headers = response_headers_storage_.data();
		hd.count = static_cast<uint32_t>(response_headers_storage_.size());

		UHR_BodyData bd = {};
		bd.body = response_body_.data();
		bd.length = static_cast<uint32_t>(response_body_.size());

		UHR_Response response;
		response.request_id = rid_;
		response.http_status = static_cast<uint32_t>(http_status_);
		response.headers = hd;
		response.body = bd;

		return response;
	}

	// static
	std::size_t Request::ReadCallback(char *ptr, std::size_t size, std::size_t nmemb, void *user_data) {
		Request *self = reinterpret_cast<Request*>(user_data);
		if (self->cancelled_)
			return CURL_READFUNC_ABORT;
		std::size_t chunk = std::min(size + nmemb, self->request_body_.size() - self->request_body_cursor_);
		memcpy(ptr, self->request_body_.data() + self->request_body_cursor_, chunk);
		self->request_body_cursor_ += chunk;
		return chunk;
	}

	// static
	std::size_t Request::WriteCallback(char *ptr, std::size_t size, std::size_t nmemb, void *user_data) {
		Request *self = reinterpret_cast<Request*>(user_data);
		if (self->cancelled_)
			return 0;
		std::size_t chunk = size * nmemb;
		self->response_body_.insert(end(self->response_body_), ptr, ptr + chunk);
		return chunk;
	}

	// static
	std::size_t Request::HeaderCallback(char *ptr, std::size_t size, std::size_t nitems, void *user_data) {
		Request *self = reinterpret_cast<Request*>(user_data);
		if (self->cancelled_)
			return 0;

		const auto len = size * nitems;
		
		// Advance index i until it reaches the colon (or the end)
		auto i = 0u;
		while (i < len && ptr[i] != ':') ++i;

		// First header line is delivered to this callback, which is, eg:  HTTP/1.1 200 OK
		// It's not really a key/value pair, so skip it if we didn't find a colon.
		if (i == len)
			return len;

		// Range [0, i) holds the header name
		auto name = ToUTF16(ptr, ptr + i);
		//UHR_LOG_INFO("Header name: '" << std::string(ptr, ptr + i) << "'");

		// Advance i past the colon, but only if not at the end
		if (i < len) ++i;

		// Advance i past any whitespace.
		// It will land on the first character of the header value (or the end)
		while (i < len && isspace(ptr[i])) ++i;
		
		// Starting at i, advance j to find the newline character (or the end)
		auto j = i;
		while (j < len && ptr[j] != 0 && ptr[j] != '\r' && ptr[j] != '\n') ++j;

		// Range [i, j) holds the value
		auto value = ToUTF16(ptr + i, ptr + j);
		//UHR_LOG_INFO("Header value: '" << std::string(ptr + i, ptr + j) << "'");

		self->response_headers_.emplace_back(std::move(name), std::move(value));
		auto &header = self->response_headers_.back();

		UHR_Header header_ref = {};
		header_ref.name.characters = reinterpret_cast<const uint16_t *>(header.first.c_str());
		header_ref.name.length = static_cast<uint32_t>(header.first.length());
		header_ref.value.characters = reinterpret_cast<const uint16_t *>(header.second.c_str());
		header_ref.value.length = static_cast<uint32_t>(header.second.length());
		self->response_headers_storage_.push_back(header_ref);

		return len;
	}

	// static
	int Request::ProgressCallback(void *user_data, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
		Request *self = reinterpret_cast<Request*>(user_data);
		if (self->cancelled_)
			return -1;
		return CURL_PROGRESSFUNC_CONTINUE;
	}

} // namespace uhr