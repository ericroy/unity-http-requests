#pragma once
#include <string>
#include <optional>
#include <curl/curl.h>
#include "UnityHttpRequests.h"

namespace uhr {

    class CACerts {
    public:
        bool Find();
        bool Apply(CURL *easy);

    private:
        std::optional<std::string> cert_bundle_;
        std::optional<std::string> cert_directory_;
    };

} // namespace uhr
