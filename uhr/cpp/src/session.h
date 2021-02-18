#pragma once
#include <memory>
#include <unordered_map>
#include <optional>
#include <curl/curl.h>
#include "UnityHttpRequests.h"
#include "request_builder.h"
#include "ca_certs.h"

namespace uhr {

	class Request;

	class Session {
	public:
		~Session();

		inline const CACerts &ca_certs() const { return certs_; }

		UHR_RequestId GetNextRid();
		bool StartRequest(std::unique_ptr<Request> req);
		bool Update();
		bool DestroyRequest(UHR_RequestId rid);
		bool GetNextNoncancelledResponse(UHR_Response *response_out);
		
		static std::unique_ptr<Session> Create();

	private:
		Session(CURLM *multi, CACerts &&certs);

		// Copy not allowed
        Session(const Session &) = delete;
        Session &operator=(const Session &) = delete;

		std::unique_ptr<Request> GetNextCompleted();
		
		UHR_RequestId next_request_id_ = 1;
		CURLM *multi_ = nullptr;
		CACerts certs_;
		
		// Requests that are in-flight
		std::unordered_map<UHR_RequestId, std::unique_ptr<Request>> requests_;
		
		// Requests that are completed *and* have loaned out references to their
		// response data to the caller.  They must sit in this map until they have
		// been explicitly destroyed by the caller.
		// Requests that were cancelled never make it into this list, they are
		// destroyed as soon as they complete.
		std::unordered_map<UHR_RequestId, std::unique_ptr<Request>> completed_;
	};

} // namespace uhr
