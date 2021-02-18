#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <locale>
#include <codecvt>
#include <atomic>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include <unordered_map>
#if !defined(_WIN32) && !defined(WIN32)
#include <sys/stat.h>
#endif
#include <curl/curl.h>
#include "UnityHttpRequests.h"

using namespace std;

namespace {

atomic_int g_init_count = 0;

u16string to_utf16(const string& s) {
	wstring_convert<codecvt_utf8_utf16<char16_t>, char16_t> converter;
	return converter.from_bytes(s);
}

string to_utf8(UHR_StringRef sr) {
	wstring_convert<codecvt_utf8_utf16<char16_t>, char16_t> convert; 
	return convert.to_bytes(
		reinterpret_cast<const char16_t *>(sr.characters),
		reinterpret_cast<const char16_t *>(sr.characters) + sr.length
	);
}

const unordered_map<UHR_Error, const u16string> g_error_strings = {
	{ UHR_ERR_OK,                         to_utf16("Ok") },
	{ UHR_ERR_INVALID_CONTEXT,            to_utf16("The context handle was invalid") },
	{ UHR_ERR_MISSING_REQUIRED_PARAMETER, to_utf16("A required function parameter was missing or null") },
	{ UHR_ERR_INVALID_HTTP_METHOD,        to_utf16("Invalid HTTP method") },
	{ UHR_ERR_FAILED_TO_CREATE_REQUEST,   to_utf16("Failed to create request") },
	{ UHR_ERR_UNKNOWN_ERROR_CODE,         to_utf16("Unknown error code") },
	{ UHR_ERR_FAILED_TO_CREATE_CONTEXT,   to_utf16("Failed to create context") },
	{ UHR_ERR_FAILED_TO_DESTROY_CONTEXT,  to_utf16("Failed to destroy context") },
	{ UHR_ERR_FAILED_TO_UPDATE_CONTEXT,   to_utf16("Failed to update context") },
};

const unordered_map<UHR_Method, const string> g_method_strings = {
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

void init() {
	if (atomic_fetch_add_explicit(&g_init_count, 1, memory_order_relaxed) == 0) {
		curl_global_init(CURL_GLOBAL_DEFAULT);
	}
}

void deinit() {
	if (atomic_fetch_sub_explicit(&g_init_count, 1, memory_order_relaxed) == 1) {
		curl_global_cleanup();
	}
}

#if !defined(_WIN32) && !defined(WIN32)
// These search paths are borrowed from the Golang implementation: https://golang.org/src/crypto/x509/root_linux.go
void load_roots(CURL *curl) {
	static const string cert_files[] = {
		"/etc/ssl/certs/ca-certificates.crt",                // Debian/Ubuntu/Gentoo etc.
		"/etc/pki/tls/certs/ca-bundle.crt",                  // Fedora/RHEL 6
		"/etc/ssl/ca-bundle.pem",                            // OpenSUSE
		"/etc/pki/tls/cacert.pem",                           // OpenELEC
		"/etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem", // CentOS/RHEL 7
		"/etc/ssl/cert.pem",                                 // Alpine Linux
	};

	static const string cert_directories[] = {
		"/etc/ssl/certs",               // SLES10/SLES11, https://golang.org/issue/12139
		"/etc/pki/tls/certs",           // Fedora/RHEL
		"/system/etc/security/cacerts", // Android
	};

	struct stat sb = {};

	for (const auto &file : cert_files) {
		if (stat(file.c_str(), &sb) == 0 && !S_ISDIR(sb.st_mode)) {
			curl_easy_setopt(curl, CURLOPT_CAINFO, file.c_str());
			cout << "Found root ca certs file: " << file << endl;
			return;
		}
	}
	
	for (const auto &folder : cert_directories) {
		if (stat(folder.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
			curl_easy_setopt(curl, CURLOPT_CAPATH, folder.c_str());
			cout << "Found root ca certs folder: " << folder << endl;
			return;
		}
	}

	cout << "No root certificates found, ssl cert verification will fail" << endl;
}
#endif

class Context;

class Request {
	friend class Context;
public:
	~Request() { Close(); }

	void Close() {
		if (easy_ != nullptr) {
			curl_easy_cleanup(easy_);
			easy_ = nullptr;
		}
		if (request_headers_list_ != nullptr) {
			curl_slist_free_all(request_headers_list_);
			request_headers_list_ = nullptr;
		}
	}

	bool SetMethod(const string &method) {
		method_ = method;
		return curl_easy_setopt(easy_, CURLOPT_CUSTOMREQUEST, method_.c_str()) == CURLE_OK;
	}

	bool SetUrl(const string &url) {
		url_ = url;
		return curl_easy_setopt(easy_, CURLOPT_URL, url_.c_str()) == CURLE_OK;
	}

	bool SetBody(const char *body, size_t body_length) {
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

	bool AddHeader(const string &name, const string &value) {
		stringstream ss;
		ss << name << ": " << value;
		request_headers_.push_back(ss.str());
	}

	void GetResponse(UHR_Response *response_out) {
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

	inline UHR_RequestId rid() const { return rid_; }
	inline uint32_t http_status() const { return http_status_; }
	inline bool completed() const { return completed_; }
	inline bool cancelled() const { return cancelled_; }

private:
	Request(UHR_RequestId rid, CURL *easy) : rid_(rid), easy_(easy) {}

	bool Prepare() {
		for (const auto &header : request_headers_)
			request_headers_list_ = curl_slist_append(request_headers_list_, header.c_str());
		if (curl_easy_setopt(easy_, CURLOPT_HTTPHEADER, request_headers_list_) != CURLE_OK)
			return false;
		return true;
	}

	void Cancel() {
		cancelled_ = true;
	}

	void Complete() {
		completed_ = true;

		long status = 0;
		http_status_ = 0;
		if (curl_easy_getinfo(easy_, CURLINFO_RESPONSE_CODE, &status) == CURLE_OK)
			http_status_ = static_cast<uint32_t>(status);
	}

	static size_t ReadCallback(char *ptr, size_t size, size_t nmemb, void *user_data) {
		Request *self = reinterpret_cast<Request*>(user_data);
		if (self->cancelled_)
			return CURL_READFUNC_ABORT;
		size_t chunk = min(size + nmemb, self->request_body_.size() - self->request_body_cursor_);
		memcpy(ptr, self->request_body_.data() + self->request_body_cursor_, chunk);
		self->request_body_cursor_ += chunk;
		return chunk;
	}

	static size_t WriteCallback(char *ptr, size_t size, size_t nmemb, void *user_data) {
		Request *self = reinterpret_cast<Request*>(user_data);
		if (self->cancelled_)
			return 0;
		size_t chunk = size * nmemb;
		self->response_body_.insert(end(self->response_body_), ptr, ptr + chunk);
		return chunk;
	}

	static size_t HeaderCallback(char *ptr, size_t size, size_t nitems, void *user_data) {
		Request *self = reinterpret_cast<Request*>(user_data);
		if (self->cancelled_)
			return 0;

		string all(ptr, size * nitems);
		
		size_t name_start = 0;
		size_t name_end = all.find_first_of(':');
		if (name_end == string::npos) {
			name_end = all.size();
		}

		size_t value_start = name_end;
		value_start = all.find_first_not_of(" \t\n\v\f\r", name_end + 1);
		size_t value_end = all.find_first_of("\n", value_start);
		if (value_end == string::npos) {
			value_end = all.size();
		}

		self->response_headers_.emplace_back(
			to_utf16(all.substr(name_start, name_end - name_start)),
			to_utf16(all.substr(value_start, value_end - value_start))
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

	static int ProgressCallback(void *user_data, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
		Request *self = reinterpret_cast<Request*>(user_data);
		if (self->cancelled_)
			return -1;
		return CURL_PROGRESSFUNC_CONTINUE;
	}


	UHR_RequestId rid_ = 0;
	CURL *easy_ = nullptr;
	curl_slist *request_headers_list_ = nullptr;

	// Backing data referenced by the easy handle
	string method_;
	string url_;
	vector<char> request_body_;
	vector<string> request_headers_;
	vector<char> response_body_;
	vector<pair<u16string, u16string>> response_headers_;
	vector<UHR_Header> response_headers_storage_;

	size_t request_body_cursor_ = 0;
	uint32_t http_status_ = 0;
	bool completed_ = false;
	bool cancelled_ = false;
};

struct Context {
	static unique_ptr<Context> Create() {
		CURLM *multi = curl_multi_init();
		if (multi == nullptr)
			return nullptr;
		return unique_ptr<Context>(new Context(multi));
	}
	
	~Context() {
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

	unique_ptr<Request> CreateRequest() {
		CURL *easy = curl_easy_init();
		if (easy == nullptr)
			return nullptr;
		
		auto rid = next_request_id_++;
		if (next_request_id_ == 0)
			next_request_id_ = 1;

		unique_ptr<Request> req(new Request(rid, easy));
		if (curl_easy_setopt(easy, CURLOPT_PRIVATE, req.get()) != CURLE_OK)
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

	bool StartRequest(unique_ptr<Request> req) {
		if (!req->Prepare())
			return false;

		if (curl_multi_add_handle(multi_, req->easy_) != CURLM_OK)
			return false;

		requests_[req->rid()] = move(req);
	}

	bool Update() {
		int still_running = 0;
		return curl_multi_perform(multi_, &still_running) == CURLM_OK;
	}

	bool DestroyRequest(UHR_RequestId rid) {
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

	Request *GetNextCompleted() {
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

private:
	Context(CURLM *multi) : multi_(multi) {}
	
	CURLM *multi_ = nullptr;
	unordered_map<UHR_RequestId, unique_ptr<Request>> requests_;
	unordered_map<UHR_RequestId, unique_ptr<Request>> cancelling_;
	UHR_RequestId next_request_id_ = 1;
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
	if (http_context_handle_out == nullptr) {
		return UHR_ERR_MISSING_REQUIRED_PARAMETER;
	}
	
	// If we leave this function without returning a valid
	// context, then we must deinit to balance.  Otherwise,
	// the deinit will be called when they destroy their handle.
	init();

	auto context = Context::Create();
	if (context == nullptr) {
		deinit();
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
	
	Context *context = reinterpret_cast<Context *>(http_context_handle);
	delete context;
	deinit();
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
	if (method_iter == end(g_method_strings))
		return UHR_ERR_INVALID_HTTP_METHOD;

	Context *context = reinterpret_cast<Context *>(http_context_handle);
	auto req = context->CreateRequest();
	
	if (!req->SetMethod(method_iter->second))
		return UHR_ERR_FAILED_TO_CREATE_REQUEST;

	if (!req->SetUrl(to_utf8(url)))
		return UHR_ERR_FAILED_TO_CREATE_REQUEST;

	if (body_length > 0 && !req->SetBody(body, body_length))
		return UHR_ERR_FAILED_TO_CREATE_REQUEST;

	if (headers_count > 0) {
		for (auto i = 0u; i < headers_count; ++i) {
			if (!req->AddHeader(to_utf8(headers[i].name), to_utf8(headers[i].value)))
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

	Context *context = reinterpret_cast<Context *>(http_context_handle);

	if (!context->Update())
		return UHR_ERR_FAILED_TO_UPDATE_CONTEXT;

	uint32_t count = 0;
	Request *req = nullptr;
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

	Context *context = reinterpret_cast<Context *>(http_context_handle);

	for (auto i = 0u; i < request_ids_count; ++i) {
		context->DestroyRequest(request_ids[0]);
	}

	return UHR_ERR_OK;
}

UHR_StringRef to_ref(const u16string &s) {
	UHR_StringRef sr = {};
	sr.characters = reinterpret_cast<const uint16_t *>(s.c_str());
	sr.length = s.size();
	return sr;
}

int main(void) {
	UHR_HttpContext context = 0;
	UHR_RequestId rid = 0;
	UHR_Error err = UHR_ERR_OK;
	auto url = to_utf16("https://example.com");
	UHR_Response responses[8] = {};
	uint32_t responses_ready = 0;
	UHR_RequestId request_ids[8] = {};
	
	err = UHR_CreateHTTPContext(&context);
	if (err != UHR_ERR_OK) {
		UHR_StringRef sr = {};
		UHR_ErrorToString(err, &sr);
		cout << to_utf8(sr) << endl;
		goto done;
	}

	cout << "Created context" << endl;

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
		cout << to_utf8(sr) << endl;
		goto done;
	}

	cout << "Request created" << endl;

	while (true) {
		err = UHR_Update(context, responses, sizeof(responses) / sizeof(responses[0]), &responses_ready);
		if (err != UHR_ERR_OK) {
			UHR_StringRef sr = {};
			UHR_ErrorToString(err, &sr);
			cout << to_utf8(sr) << endl;
			goto done;
		}

		if (responses_ready > 0) {
			auto &response = responses[0];
			cout << "BODY:" << endl;
			cout << string(response.body.body, response.body.length) << endl;

			request_ids[0] = rid;
			err = UHR_DestroyRequests(context, request_ids, 1);
			if (err != UHR_ERR_OK) {
				UHR_StringRef sr = {};
				UHR_ErrorToString(err, &sr);
				cout << to_utf8(sr) << endl;
				goto done;
			}
			break;
		}

		this_thread::sleep_for(chrono::milliseconds(100));
	}


done:
	UHR_DestroyHTTPContext(context);
	cout << "Destroyed context" << endl;
	return 0;
}