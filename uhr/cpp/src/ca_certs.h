#pragma once
#include <string>
#include <curl/curl.h>
#include "UnityHttpRequests.h"

namespace uhr {

    class CACerts {
    public:
        CACerts() = default;
        CACerts(CACerts &&) = default;
        bool Find();
        bool Apply(CURL *easy) const;

    private:
        // Copy not allowed
        CACerts(const CACerts &) = delete;
        CACerts &operator=(const CACerts &) = delete;

        std::string cert_bundle_;
        std::string cert_directory_;
    };

} // namespace uhr
