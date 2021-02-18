#pragma once
#include <memory>
#include <unordered_map>
#include <curl/curl.h>
#include "UnityHttpRequests.h"
#include "ca_certs.h"

namespace uhr {

	class Request;

	class Context {
	public:
		~Context();
		std::unique_ptr<Request> CreateRequest();
		bool StartRequest(std::unique_ptr<Request> req);
		bool Update();
		bool DestroyRequest(UHR_RequestId rid);
		Request *GetNextCompleted();
		
		static std::unique_ptr<Context> Create();

	private:
		Context(CURLM *multi, CACerts certs);
		
		CURLM *multi_ = nullptr;
		CACerts certs_;
		std::unordered_map<UHR_RequestId, std::unique_ptr<Request>> requests_;
		std::unordered_map<UHR_RequestId, std::unique_ptr<Request>> cancelling_;
		UHR_RequestId next_request_id_ = 1;
	};

} // namespace uhr
