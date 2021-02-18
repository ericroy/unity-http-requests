#include "request_builder.h"
#include <curl/curl.h>
#include "request.h"

namespace uhr {

	void RequestBuilder::SetMethod(const std::string &method) {
		if (req_)
			req_->method_ = method;
	}

	void RequestBuilder::SetUrl(const std::string &url) {
		if (req_)
			req_->url_ = url;
	}

	void RequestBuilder::SetBody(const char *body, size_t body_length) {
		if (req_) {
			req_->request_body_.reserve(body_length);
			req_->request_body_.assign(body, body + body_length);
		}
	}

	void RequestBuilder::AddHeader(const std::string &name, const std::string &value) {
		static const std::string sep(": ");
		if (req_) {
			std::string s;
			s.reserve(name.length() + sep.length() + value.length());
			s += name;
			s += sep;
			s += value;
			req_->request_headers_.push_back(std::move(s));
		}
	}

	std::unique_ptr<Request> RequestBuilder::Build() {
		if (!req_)
			return nullptr;

		if (curl_easy_setopt(req_->easy_, CURLOPT_CUSTOMREQUEST, req_->method_.c_str()) != CURLE_OK)
			return nullptr;

		if (curl_easy_setopt(req_->easy_, CURLOPT_URL, req_->url_.c_str()) != CURLE_OK)
			return nullptr;

		if (req_->request_body_.size() > 0) {
			if (curl_easy_setopt(req_->easy_, CURLOPT_UPLOAD, 1L) != CURLE_OK)
				return nullptr;
			if (curl_easy_setopt(req_->easy_, CURLOPT_READFUNCTION, Request::ReadCallback) != CURLE_OK)
				return nullptr;
			if (curl_easy_setopt(req_->easy_, CURLOPT_READDATA, req_.get()) != CURLE_OK)
				return nullptr;
			if (curl_easy_setopt(req_->easy_, CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(req_->request_body_.size())) != CURLE_OK)
				return nullptr;
		}

		if (!req_->request_headers_.empty()) {
			for (const auto &header : req_->request_headers_)
				req_->request_headers_list_ = curl_slist_append(req_->request_headers_list_, header.c_str());
			
			if (curl_easy_setopt(req_->easy_, CURLOPT_HTTPHEADER, req_->request_headers_list_) != CURLE_OK)
				return nullptr;
		}
		
		return std::move(req_);
	}

	// static
	std::optional<RequestBuilder> RequestBuilder::Create(UHR_RequestId rid, const CACerts &certs) {
		CURL *easy = curl_easy_init();
		if (easy == nullptr)
			return std::nullopt;

		// Request owns the easy handle now, and will clean it up on destruction.
		auto req = std::unique_ptr<Request>(new Request(rid, easy));

		if (curl_easy_setopt(req->easy_, CURLOPT_PRIVATE, req.get()) != CURLE_OK)
			return std::nullopt;

		if (!certs.Apply(req->easy_))
			return std::nullopt;
	   
		if (curl_easy_setopt(req->easy_, CURLOPT_WRITEFUNCTION, Request::WriteCallback) != CURLE_OK)
			return std::nullopt;

		if (curl_easy_setopt(req->easy_, CURLOPT_WRITEDATA, req.get()) != CURLE_OK)
			return std::nullopt;

		if (curl_easy_setopt(req->easy_, CURLOPT_HEADERFUNCTION, Request::HeaderCallback) != CURLE_OK)
			return std::nullopt;

		if (curl_easy_setopt(req->easy_, CURLOPT_HEADERDATA, req.get()) != CURLE_OK)
			return std::nullopt;

		if (curl_easy_setopt(req->easy_, CURLOPT_XFERINFOFUNCTION, Request::ProgressCallback) != CURLE_OK)
			return std::nullopt;

		if (curl_easy_setopt(req->easy_, CURLOPT_XFERINFODATA, req.get()) != CURLE_OK)
			return std::nullopt;
		
		// Builder owns the request now
		return RequestBuilder(std::move(req));
	}

	RequestBuilder::RequestBuilder(std::unique_ptr<Request> &&req) : req_(std::forward<decltype(req)>(req)) {}

} // namespace uhr