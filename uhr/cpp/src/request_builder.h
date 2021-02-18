#pragma once
#include <memory>
#include <string>
#include <optional>
#include "UnityHttpRequests.h"
#include "ca_certs.h"

namespace uhr {

	class Request;

	class RequestBuilder {
	public:
        RequestBuilder(RequestBuilder &&) = default;
        void SetMethod(const std::string &method);
		void SetUrl(const std::string &url);
		void SetBody(const char *body, size_t body_length);
		void AddHeader(const std::string &name, const std::string &value);
		std::unique_ptr<Request> Build();

        static std::optional<RequestBuilder> Create(UHR_RequestId rid, const CACerts &certs);
	private:
        RequestBuilder(std::unique_ptr<Request> &&req);
        
        // Copy not allowed
        RequestBuilder(const RequestBuilder &) = delete;
        RequestBuilder &operator=(const RequestBuilder &) = delete;

		std::unique_ptr<Request> req_;
	};

} // namespace uhr
