#include "session.h"
#include "logging.h"
#include "request.h"

namespace uhr {

	Session::~Session() {
		// Cancel any running requests
		for (auto &kv : requests_)
			kv.second->set_cancelled();

		// While there are still cancelled requests waiting to complete
		// and our Update (curl_multi_perform) call is succeeding, keep waiting.
		while (!requests_.empty() && Update()) {
			// Pull out each completed request, and abandon it so it
			// goes out of scope and is deleted.
			while (GetNextCompleted() != nullptr) {}
		}

		auto res = curl_multi_cleanup(multi_);
		if (res != CURLM_OK) {
			UHR_LOG_CRITICAL("curl_multi_cleanup failed: " << curl_multi_strerror(res));
		}
	}

	UHR_RequestId Session::GetNextRid() {
		auto rid = next_request_id_++;
		if (next_request_id_ == 0)
			next_request_id_ = 1;
		return rid;
	}

	bool Session::StartRequest(std::unique_ptr<Request> req) {
		if (!req->Attach(multi_))
			return false;

		requests_[req->rid()] = move(req);
		return true;
	}

	bool Session::Update() {
		int still_running = 0;
		return curl_multi_perform(multi_, &still_running) == CURLM_OK;
	}

	bool Session::DestroyRequest(UHR_RequestId rid) {
		// First check the most common case, which is that this is a completed
		// request whose response data we had loaned out to the caller.
		// This is the caller telling us that they're done with the data now.
		auto iter = completed_.find(rid);
		if (iter != std::end(completed_)) {
			completed_.erase(iter);
			return true;
		}

		// Then check the less common case; that the caller is cancelling
		// an in-flight request.  This means they don't want to find out
		// about the result of it later.
		iter = requests_.find(rid);
		if (iter != std::end(requests_)) {
			iter->second->set_cancelled();
			return true;
		}

		return false;
	}

	bool Session::GetNextNoncancelledResponse(UHR_Response *response_out) {
		// For each of the requests that we receive, ownership is transferred to this
		// scope.  If we let it go out of scope (or move a new value into the unique_ptr)
		// we are causing the destruction of the Request.
		for (auto req = GetNextCompleted(); req != nullptr; req = GetNextCompleted()) {
			// If it was cancelled, simply abandon it so it gets destructed.
			if (req->cancelled())
				continue;

			// Loan out references to the response data.
			// This means we must keep the Request around as long as it is on loan.
			*response_out = req->GetResponse();

			// Completed map now owns the request
			completed_[req->rid()] = std::move(req);

			return true;
		}
		return false;
	}

	// static
	std::unique_ptr<Session> Session::Create() {
		CURLM *multi = curl_multi_init();
		if (multi == nullptr)
			return nullptr;

		CACerts certs;
		if (!certs.Find())
			return nullptr;

		return std::unique_ptr<Session>(new Session(multi, std::move(certs)));
	}

	Session::Session(CURLM *multi, CACerts &&certs) : multi_(multi), certs_(std::forward<decltype(certs)>(certs)) {
	}

	std::unique_ptr<Request> Session::GetNextCompleted() {
		int remaining_in_queue = 0;
		CURLMsg *msg = nullptr;
		while ((msg = curl_multi_info_read(multi_, &remaining_in_queue)) != nullptr) {
			if(msg->msg != CURLMSG_DONE)
				continue;
			// All easy handles must have a Request userdata.
			void *user_data;
			if (curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &user_data) != CURLE_OK) {
				UHR_LOG_CRITICAL("Easy handle had no Request userdata");
				continue;
			}
			
			auto rid = reinterpret_cast<Request *>(user_data)->rid();
			auto iter = requests_.find(rid);
			if (iter == std::end(requests_)) {
				UHR_LOG_CRITICAL("Request not found in in-flight requests list");
				continue;
			}
			
			// This scope now owns the Request
			auto req = std::move(iter->second);
			requests_.erase(iter);

			req->OnComplete();

			// Dissociate easy handle from multi handle
			req->Detach(multi_);
			
			// Close the easy handle, free request-related memory.
			// Keep response-related data though.
			req->Cleanup();

			// Transfer ownership of Request to caller, move semantics are used.
			return req;
		}

		return nullptr;
	}

} // namespace uhr