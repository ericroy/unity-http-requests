#include "context.h"
#include "request.h"

namespace uhr {

	Context::~Context() {
		// Cancel any running requests
		while (!requests_.empty()) {
			auto iter = begin(requests_);
			auto rid = iter->first;
			auto req = move(iter->second);
			requests_.erase(iter);
			req->Cancel();
			cancelling_[rid] = move(req);
		}

		// While there are still cancelled requests waiting to complete
		// and our Update (curl_multi_perform) call is succeeding, keep waiting.
		while (!cancelling_.empty() && Update()) {
			Request *req = nullptr;
			while ((req = GetNextCompleted()) != nullptr) {
				auto iter = cancelling_.find(req->rid());
				if (iter != end(cancelling_)) {

					// Dissociate easy handle from multi handle.
					// The Request will close the easy handle as it destructs.
					if (curl_multi_remove_handle(multi_, iter->second->easy_) != CURLM_OK) {
						// Uh oh...
					}

					// This destroys the Request
					cancelling_.erase(iter);
				}
			}
		}

		if (curl_multi_cleanup(multi_) != CURLM_OK) {
			// Uh oh...
		}
	}

	std::unique_ptr<Request> Context::CreateRequest() {
		CURL *easy = curl_easy_init();
		if (easy == nullptr)
			return nullptr;

		auto rid = next_request_id_++;
		if (next_request_id_ == 0)
			next_request_id_ = 1;


		std::unique_ptr<Request> req(new Request(rid, easy));
		if (curl_easy_setopt(easy, CURLOPT_PRIVATE, req.get()) != CURLE_OK)
			return nullptr;
		
		if (!certs_.Apply(req->easy_))
			return nullptr;
		
		if (curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, Request::WriteCallback) != CURLE_OK)
			return nullptr;
		if (curl_easy_setopt(easy, CURLOPT_WRITEDATA, req.get()) != CURLE_OK)
			return nullptr;
		
		if (curl_easy_setopt(easy, CURLOPT_HEADERFUNCTION, Request::HeaderCallback) != CURLE_OK)
			return nullptr;
		if (curl_easy_setopt(easy, CURLOPT_HEADERDATA, req.get()) != CURLE_OK)
			return nullptr;

		if (curl_easy_setopt(easy, CURLOPT_XFERINFOFUNCTION, Request::ProgressCallback) != CURLE_OK)
			return nullptr;
		if (curl_easy_setopt(easy, CURLOPT_XFERINFODATA, req.get()) != CURLE_OK)
			return nullptr;

		return req;
	}

	bool Context::StartRequest(std::unique_ptr<Request> req) {
		if (!req->Prepare())
			return false;

		if (curl_multi_add_handle(multi_, req->easy_) != CURLM_OK)
			return false;

		requests_[req->rid()] = move(req);
		return true;
	}

	bool Context::Update() {
		int still_running = 0;
		return curl_multi_perform(multi_, &still_running) == CURLM_OK;
	}

	bool Context::DestroyRequest(UHR_RequestId rid) {
		auto iter = requests_.find(rid);
		if (iter == end(requests_))
			return false;
		
		// Request is now owned in this scope
		auto req = move(iter->second);

		requests_.erase(iter);

		if (!req->completed()) {
			req->Cancel();
			// Request is now owned by cancelling_ map
			cancelling_[rid] = move(req);
		} else {
			// Dissociate easy handle from multi handle.
			// The Request will close the easy handle as it goes out of scope.
			if (curl_multi_remove_handle(multi_, req->easy_) != CURLM_OK)
				return false;
		}
		return true;
	}

	Request *Context::GetNextCompleted() {
		int remaining_in_queue = 0;
		CURLMsg *msg = nullptr;
		while ((msg = curl_multi_info_read(multi_, &remaining_in_queue)) != nullptr) {
			if(msg->msg == CURLMSG_DONE) {
				// All easy handles must have a Request userdata.
				void *user_data;
				curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &user_data);
				Request *req = reinterpret_cast<Request *>(user_data);
				req->Complete();
				return req;
			}
		}
		return nullptr;
	}

	// static
	std::unique_ptr<Context> Context::Create() {
		CURLM *multi = curl_multi_init();
		if (multi == nullptr)
			return nullptr;

		CACerts certs;
		if (!certs.Find())
			return nullptr;

		return std::unique_ptr<Context>(new Context(multi, std::move(certs)));
	}

	Context::Context(CURLM *multi, CACerts certs) : multi_(multi), certs_(certs) {

	}

} // namespace uhr