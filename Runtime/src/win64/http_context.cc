#include "http_context.h"
#include <sstream>
#include <Windows.h>
#include <comutil.h>

namespace {
    const int kResolveTimeout = 5000;
    const int kConnectTimeout = 5000;
    const int kSendTimeout = 5000;
    const int kReceiveTimeout = 5000;
}

HttpContext::HttpContext() {
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
}

HttpContext::~HttpContext() {
    CoUninitialize();
}

UHR_RequestId HttpContext::CreateRequest(
    UHR_StringRef url,
    int method,
    UHR_Header *request_headers,
    int request_headers_count,
    const char *request_body,
    int request_body_length,
    std::u16string *error
) {
    IWinHttpRequestPtr req = nullptr;
    auto hr = req.CreateInstance(__uuidof(WinHttpRequest));
    if (FAILED(hr)) {
        *error = u"CreateInstance() failed";
        return UHR_REQUEST_ID_INVALID;
    }

    hr = req->SetTimeouts(kResolveTimeout, kConnectTimeout, kSendTimeout, kReceiveTimeout);
    if (FAILED(hr)) {
        *error = u"SetTimeouts() failed";
        return UHR_REQUEST_ID_INVALID;
    }

    _bstr_t bmethod(method == UHR_METHOD_GET ? u"GET" : u"POST");
    _bstr_t burl(::SysAllocStringLen(url.characters, url.length), false);
    _variant_t async(true);
    hr = req->Open(bmethod, burl, async);
	if (FAILED(hr)) {
        *error = u"Open() failed";
        return UHR_REQUEST_ID_INVALID;
    }

    for (int i = 0; i < request_headers_count; ++i) {
        auto& header = request_headers[i];
        _bstr_t name(::SysAllocStringLen(header.name.characters, header.name.length), false);
        _bstr_t value(::SysAllocStringLen(header.value.characters, header.value.length), false);
        req->SetRequestHeader(name, value);
    }

    _variant_t body;
    if (request_body_length > 0) {
        body.vt = VT_ARRAY | VT_UI1;
        
        SAFEARRAYBOUND rgsabound[1];
        rgsabound[0].cElements = request_body_length;
        rgsabound[0].lLbound = 0;
        body.parray = SafeArrayCreate(VT_UI1, 1, rgsabound);

        void *dst = nullptr;    
        SafeArrayAccessData(body.parray, &dst);
        memcpy(dst, request_body, request_body_length); 
        SafeArrayUnaccessData(body.parray);
    }

    hr = req->Send(body);
	if (FAILED(hr)) {
        *error = u"Open() failed";
        return UHR_REQUEST_ID_INVALID;
    }

    auto rid = next_request_id_++;
    if (next_request_id_ < 0)
        next_request_id_ = 1;

    Request request;
    request.rid = rid;
    request.req = std::move(req);
    requests_.insert(std::move(request));

    return rid;
}

int HttpContext::Update(UHR_HttpContext handle, UHR_Response* responses_out, int responses_capacity, std::u16string *error) {
    static const _variant_t timeout(0);

    int output_cursor = 0;
    auto iter = std::begin(requests_);
    while (iter != end(requests_) && output_cursor < responses_capacity) {
        VARIANT_BOOL ready = VARIANT_FALSE;
        auto hr = iter->req->WaitForResponse(timeout, &ready);
        if (FAILED(hr)) {
            *error = u"WaitForResponse() failed";
            return -1;
        }

        // If it's still in progress, continue
        if (ready == VARIANT_FALSE) {
            ++iter;
            continue;
        }

        _bstr_t headers;
        hr = iter->req->GetAllResponseHeaders(&headers);
        if (FAILED(hr)) {
            *error = u"GetAllResponseHeaders() failed";
            return -1;
        }

        _bstr_t body;
        hr = iter->req->get_ResponseText(&body);
        if (FAILED(hr)) {
            *error = u"get_ResponseText() failed";
            return -1;
        }

        long http_status = -1;
        hr = iter->req->get_Status(&http_status);
        if (FAILED(hr)) {
            *error = u"get_Status() failed";
            return -1;
        }

        UHR_Response& res = responses_out[output_cursor++];
        res.request_id = iter->rid;
        res.http_status = static_cast<int>(http_status);
        res.response_body = iter->response_body.data();
        res.response_body_length = static_cast<int>(iter->response_body.size());

        // Parse response headers
        res.response_headers_count = 0;
        std::basic_stringstream<char16_t> ss(bs, SysStringLen(bs));
        for (std::u16string line; std::getline(ss, line); ) {
            auto colon = line.find(u':');
            if (colon != std::u16string::npos) {
                // Populate the backing memory
                iter->response_headers.emplace_back(line.sub(0, colon), line.sub(colon + 1));
                auto& pair = iter->response_headers.back();

                // Get the borrowed pointers to return
                auto& out = res.response_headers[res.response_headers_count++];
                out.name = UHR_StringRef{ pair.first.c_str(), static_cast<int>(pair.first.length()) };
                out.value = UHR_StringRef{ pair.second.c_str(), static_cast<int>(pair.second.length()) };
            }
        }

        // Move it to the completed list, and set iterator to next
        completed_.insert(std::move(*iter));
        iter = requests_.erase(iter);
    }

    return output_cursor;
}

int HttpContext::DestroyRequests(UHR_HttpContext handle, UHR_RequestId* request_ids, int request_ids_count, std::u16string *error) {
    for (auto i = 0; i < request_ids_count; ++i) {
        bool found = false;
        // Check completed list first, as it's most common to delete a completed request
        for (auto &req : completed_) {
            if (req.request_id == request_ids[i]) {
                std::swap(req, completed_.back());
                completed_.pop_back();
                found = true;
                break;
            }
        }
        
        if (found) {
            continue;
        }

        for (auto &req : requests_) {
            if (req.request_id == request_ids[i]) {
                std::swap(req, requests_.back());
                requests_.pop_back();
                found = true;
                break;
            }
        }

        if (!found) {
            *error = u"A request id was not found";
            return -1;
        }
    }
    return 0;
}