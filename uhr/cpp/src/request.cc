#include "request.h"
#include <cstring>
#include <sstream>
#include "util.h"

namespace uhr {

	Request::~Request() {
		Close();
	}

	void Request::Close() {
		if (easy_ != nullptr) {
			curl_easy_cleanup(easy_);
			easy_ = nullptr;
		}
		if (request_headers_list_ != nullptr) {
			curl_slist_free_all(request_headers_list_);
			request_headers_list_ = nullptr;
		}
	}

	bool Request::SetMethod(const std::string &method) {
		method_ = method;
		return curl_easy_setopt(easy_, CURLOPT_CUSTOMREQUEST, method_.c_str()) == CURLE_OK;
	}

	bool Request::SetUrl(const std::string &url) {
		url_ = url;
		return curl_easy_setopt(easy_, CURLOPT_URL, url_.c_str()) == CURLE_OK;
	}

	bool Request::SetBody(const char *body, size_t body_length) {
		request_body_.reserve(body_length);
		request_body_.assign(body, body + body_length);
		if (curl_easy_setopt(easy_, CURLOPT_UPLOAD, 1L) != CURLE_OK)
			return false;
		if (curl_easy_setopt(easy_, CURLOPT_READFUNCTION, Request::ReadCallback) != CURLE_OK)
			return false;
		if (curl_easy_setopt(easy_, CURLOPT_READDATA, this) != CURLE_OK)
			return false;
		if (curl_easy_setopt(easy_, CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(request_body_.size())) != CURLE_OK)
			return false;
		return true;
	}

	bool Request::AddHeader(const std::string &name, const std::string &value) {
		std::stringstream ss;
		ss << name << ": " << value;
		request_headers_.push_back(ss.str());
	}

	void Request::GetResponse(UHR_Response *response_out) {
		UHR_HeadersData hd = {};
		hd.headers = response_headers_storage_.data();
		hd.count = static_cast<uint32_t>(response_headers_storage_.size());

		UHR_BodyData bd = {};
		bd.body = response_body_.data();
		bd.length = static_cast<uint32_t>(response_body_.size());

		response_out->request_id = rid_;
		response_out->http_status = static_cast<uint32_t>(http_status_);
		response_out->headers = hd;
		response_out->body = bd;
	}

	Request::Request(UHR_RequestId rid, CURL *easy) : rid_(rid), easy_(easy) {}

	bool Request::Prepare() {
		for (const auto &header : request_headers_)
			request_headers_list_ = curl_slist_append(request_headers_list_, header.c_str());
		if (curl_easy_setopt(easy_, CURLOPT_HTTPHEADER, request_headers_list_) != CURLE_OK)
			return false;
		return true;
	}

	void Request::Cancel() {
		cancelled_ = true;
	}

	void Request::Complete() {
		completed_ = true;

		long status = 0;
		http_status_ = 0;
		if (curl_easy_getinfo(easy_, CURLINFO_RESPONSE_CODE, &status) == CURLE_OK)
			http_status_ = static_cast<uint32_t>(status);
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

		std::string all(ptr, size * nitems);
		
		std::size_t name_start = 0;
		std::size_t name_end = all.find_first_of(':');
		if (name_end == std::string::npos)
			name_end = all.size();

		std::size_t value_start = name_end;
		value_start = all.find_first_not_of(" \t\n\v\f\r", name_end + 1);
		std::size_t value_end = all.find_first_of("\n", value_start);
		if (value_end == std::string::npos)
			value_end = all.size();

		self->response_headers_.emplace_back(
			ToUTF16(all.substr(name_start, name_end - name_start)),
			ToUTF16(all.substr(value_start, value_end - value_start))
		);
		auto &header = self->response_headers_.back();

		UHR_Header header_ref = {};
		header_ref.name.characters = reinterpret_cast<const uint16_t *>(header.first.c_str());
		header_ref.name.length = static_cast<uint32_t>(header.first.length());
		header_ref.value.characters = reinterpret_cast<const uint16_t *>(header.second.c_str());
		header_ref.value.length = static_cast<uint32_t>(header.second.length());
		self->response_headers_storage_.push_back(header_ref);

		return all.size();
	}

	// static
	int Request::ProgressCallback(void *user_data, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
		Request *self = reinterpret_cast<Request*>(user_data);
		if (self->cancelled_)
			return -1;
		return CURL_PROGRESSFUNC_CONTINUE;
	}

} // namespace uhr