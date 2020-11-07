#pragma once

// Import the com typelib
#import "winhttpcom.dll" no_namespace

#include <string>
#include <vector>
#include <memory>
#include <comutil.h>
#include "unity_http_requests.h"

namespace unity_http_requests {
    
    class HttpContext {
    public:
        HttpContext();
        ~HttpContext();

        UHR_RequestId CreateRequest(
            UHR_StringRef url,
            int method,
            UHR_Header *request_headers,
            int request_headers_count,
            const char *request_body,
            int request_body_length,
            std::u16string *error
        );
        
        int Update(UHR_Response* responses_out, int responses_capacity, std::u16string *error);

        int DestroyRequests(UHR_RequestId* request_ids, int request_ids_count, std::u16string *error);

    private:
        struct Request {
            struct ResponseStorage {
                std::vector<std::pair<std::u16string, std::u16string>> headers_;
                std::vector<char> body_;
                int http_status_;
            };

            IWinHttpRequestPtr req_;
            std::unique_ptr<ResponseStorage> res_;
            int rid_ = UHR_REQUEST_ID_INVALID;
        };

        bool DestroyRequest(UHR_RequestId rid, std::u16string *error);

        std::vector<Request> requests_;
        std::vector<Request> completed_;
        int next_request_id_ = 1;
    };

} // namespace unity_http_requests

