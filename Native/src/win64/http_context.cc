#include "http_context.h"
#include <sstream>
#include <Windows.h>
#include <comutil.h>

namespace http_requests {

    namespace {
        const int kResolveTimeout = 5000;
        const int kConnectTimeout = 5000;
        const int kSendTimeout = 5000;
        const int kReceiveTimeout = 5000;

        _bstr_t BStrFromStringRef(const UHR_StringRef& ref) {
            return _bstr_t(::SysAllocStringLen(reinterpret_cast<const OLECHAR*>(ref.characters), ref.length), false);
        }

        UHR_StringRef StringRefFromU16String(const std::u16string& s) {
            UHR_StringRef sr;
            sr.characters = reinterpret_cast<const short*>(s.c_str());
            sr.length = s.length();
            return sr;
        }

        const char16_t* BStrToChar16Ptr(const _bstr_t& bs) {
            return reinterpret_cast<const char16_t*>(static_cast<const wchar_t*>(bs));
        }

        std::u16string operator+(std::u16string& lhs, const _com_error& err) {
            static_assert(sizeof(wchar_t) == sizeof(char16_t), "On windows, wchar_t and char16_t should be the same size");
            return lhs + std::u16string(reinterpret_cast<const char16_t*>(err.ErrorMessage()));
        }

        template<int N>
        std::u16string operator+(const char16_t (&lhs)[N], const _com_error& err) {
            static_assert(sizeof(wchar_t) == sizeof(char16_t), "On windows, wchar_t and char16_t should be the same size");
            return lhs + std::u16string(reinterpret_cast<const char16_t*>(err.ErrorMessage()));
        }
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
        
        try {
            req->SetTimeouts(kResolveTimeout, kConnectTimeout, kSendTimeout, kReceiveTimeout);
        } catch(const _com_error& ce) {
            *error = u"SetTimeouts() failed: " + ce;
            return UHR_REQUEST_ID_INVALID;
        }

        _bstr_t bmethod(method == UHR_METHOD_GET ? u"GET" : u"POST");
        _bstr_t burl = BStrFromStringRef(url);
        _variant_t async(true);
        try {
            req->Open(bmethod, burl, async);
        } catch(const _com_error& ce) {
            *error = u"Open() failed" + ce;
            return UHR_REQUEST_ID_INVALID;
        }

        for (int i = 0; i < request_headers_count; ++i) {
            auto& header = request_headers[i];
            _bstr_t name = BStrFromStringRef(header.name);
            _bstr_t value = BStrFromStringRef(header.value);
            try {
                req->SetRequestHeader(name, value);
            } catch(const _com_error& ce) {
                *error = u"SetRequestHeader() failed" + ce;
                return UHR_REQUEST_ID_INVALID;
            }
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

        try {
            req->Send(body);
        } catch(const _com_error& ce) {
            *error = u"Send() failed" + ce;
            return UHR_REQUEST_ID_INVALID;
        }

        auto rid = next_request_id_++;
        if (next_request_id_ < 0)
            next_request_id_ = 1;

        Request request;
        request.rid = rid;
        request.req = std::move(req);
        requests_.push_back(std::move(request));

        return rid;
    }

    int HttpContext::Update(UHR_Response* responses_out, int responses_capacity, std::u16string *error) {
        static const _variant_t zero_timeout(0);

        int output_cursor = 0;
        auto iter = std::begin(requests_);
        while (iter != end(requests_) && output_cursor < responses_capacity) {
            VARIANT_BOOL ready = VARIANT_FALSE;
            try {
                ready = iter->req->WaitForResponse(zero_timeout);
            } catch(const _com_error& ce) {
                *error = u"WaitForResponse() failed" + ce;
                return -1;
            }

            // If it's still in progress, continue
            if (ready == VARIANT_FALSE) {
                ++iter;
                continue;
            }

            _bstr_t headers;
            try {
                headers = iter->req->GetAllResponseHeaders();
            } catch(const _com_error& ce) {
                *error = u"GetAllResponseHeaders() failed" + ce;
                return -1;
            }

            _bstr_t body;
            try {
                body = iter->req->GetResponseText();
            } catch(const _com_error& ce) {
                *error = u"GetResponseText() failed" + ce;
                return -1;
            }

            long http_status = -1;
            try {
                http_status = iter->req->GetStatus();
            } catch(const _com_error& ce) {
                *error = u"GetStatus() failed" + ce;
                return -1;
            }

            // Get a reference to the output item we want to fill out.
            // Advance the output cursor.
            UHR_Response& res = responses_out[output_cursor++];
            res.request_id = iter->rid;
            res.http_status = static_cast<int>(http_status);
            res.response_body = iter->response_body.data();
            res.response_body_length = static_cast<int>(iter->response_body.size());

            // Parse response headers
            res.response_headers_count = 0;
            std::basic_stringstream<char16_t> ss(BStrToChar16Ptr(headers));
            for (std::u16string line; std::getline(ss, line); ) {
                auto colon = line.find(u':');
                if (colon != std::u16string::npos) {
                    // Populate the backing memory
                    iter->response_headers.emplace_back(line.substr(0, colon), line.substr(colon + 1));
                    auto& pair = iter->response_headers.back();

                    // Get the borrowed pointers to return
                    auto& out = res.response_headers[res.response_headers_count++];
                    out.name = StringRefFromU16String(pair.first);
                    out.value = StringRefFromU16String(pair.second);
                }
            }

            // Move it to the completed list, and advance iterator
            completed_.push_back(std::move(*iter));
            iter = requests_.erase(iter);
        }

        return output_cursor;
    }

    int HttpContext::DestroyRequests(UHR_RequestId* request_ids, int request_ids_count, std::u16string *error) {
        for (auto i = 0; i < request_ids_count; ++i) {
            bool found = false;
            // Check completed list first, as it's most common to delete a completed request
            for (auto &request : completed_) {
                if (request.rid == request_ids[i]) {
                    std::swap(request, completed_.back());
                    completed_.pop_back();
                    found = true;
                    break;
                }
            }
            
            if (found) {
                continue;
            }

            for (auto &request : requests_) {
                if (request.rid == request_ids[i]) {
                    std::swap(request, requests_.back());
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

} // namespace http_requests