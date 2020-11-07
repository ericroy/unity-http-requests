#pragma once

// Import the com typelib
#import "winhttpcom.dll" no_namespace

#include <string>
#include <vector>
#include "http_requests.h"

namespace http_requests {
    
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
            int rid = UHR_REQUEST_ID_INVALID;
            IWinHttpRequestPtr req;

            // These just hold the backing memory.
            // Pointers to this data are borrowed and returned to the
            // caller in the UHR_Response.
            // The UHR_Response is invalid once the request has been destroyed.
            std::vector<std::pair<std::u16string, std::u16string>> response_headers;
            std::vector<char> response_body;

            // For the std::set, so it can sort
            inline bool operator<(const Request& o) const { return rid < o.rid; }
        };

        std::vector<Request> requests_;
        std::vector<Request> completed_;
        int next_request_id_ = 1;
    };

} // namespace http_requests

