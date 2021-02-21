#include "ca_certs.h"
#if !defined(_WIN32) && !defined(WIN32)
#include <sys/stat.h>
#endif

namespace uhr {

    namespace {

    #if !defined(_WIN32) && !defined(WIN32)
        // These search paths are borrowed from the Golang implementation: https://golang.org/src/crypto/x509/root_linux.go
		const std::string cert_files[] = {
			"/etc/ssl/certs/ca-certificates.crt",                // Debian/Ubuntu/Gentoo etc.
			"/etc/pki/tls/certs/ca-bundle.crt",                  // Fedora/RHEL 6
			"/etc/ssl/ca-bundle.pem",                            // OpenSUSE
			"/etc/pki/tls/cacert.pem",                           // OpenELEC
			"/etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem", // CentOS/RHEL 7
			"/etc/ssl/cert.pem",                                 // Alpine Linux
		};

		const std::string cert_directories[] = {
			"/etc/ssl/certs",               // SLES10/SLES11, https://golang.org/issue/12139
			"/etc/pki/tls/certs",           // Fedora/RHEL
			"/system/etc/security/cacerts", // Android
		};
    #endif
    }

    bool CACerts::Find() {
    #if !defined(_WIN32) && !defined(WIN32)
        // Not necessary on windows, since libcurl uses schannel, which pulls
        // ca certs from the system store.
        struct stat sb = {};

        for (const auto &file : cert_files) {
            if (stat(file.c_str(), &sb) == 0 && !S_ISDIR(sb.st_mode)) {
                cert_bundle_ = std::make_optional(file);
                return true;
            }
        }
        
        for (const auto &directory : cert_directories) {
            if (stat(directory.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
                cert_directory_ = std::make_optional(directory);
                return true;
            }
        }

        return false;
    #else
        return true;
    #endif
    }

    bool CACerts::Apply(CURL *easy) const {
    #if !defined(_WIN32) && !defined(WIN32)
        if (cert_bundle_)
            return curl_easy_setopt(easy, CURLOPT_CAINFO, cert_bundle_.value().c_str()) == CURLE_OK;
        if (cert_directory_)
            return curl_easy_setopt(easy, CURLOPT_CAPATH, cert_directory_.value().c_str()) == CURLE_OK;
        return false;
    #else
        (void)easy;
        return true;
    #endif
    }

} // namespace uhr