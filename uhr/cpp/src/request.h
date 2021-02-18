#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <curl/curl.h>
#include "UnityHttpRequests.h"

namespace uhr {

	class Context;

	class Request {
		friend class Context;
	public:
		~Request();
		void Close();
		bool SetMethod(const std::string &method);
		bool SetUrl(const std::string &url);
		bool SetBody(const char *body, size_t body_length);
		bool AddHeader(const std::string &name, const std::string &value);
		void GetResponse(UHR_Response *response_out);

		inline UHR_RequestId rid() const { return rid_; }
		inline std::uint32_t http_status() const { return http_status_; }
		inline bool completed() const { return completed_; }
		inline bool cancelled() const { return cancelled_; }

	private:
		Request(UHR_RequestId rid, CURL *easy);
		bool Prepare();
		void Cancel();
		void Complete();

		static std::size_t ReadCallback(char *ptr, std::size_t size, std::size_t nmemb, void *user_data);
		static std::size_t WriteCallback(char *ptr, std::size_t size, std::size_t nmemb, void *user_data);
		static std::size_t HeaderCallback(char *ptr, std::size_t size, std::size_t nitems, void *user_data);
		static int ProgressCallback(void *user_data, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

		UHR_RequestId rid_ = 0;
		CURL *easy_ = nullptr;
		curl_slist *request_headers_list_ = nullptr;

		// Backing data referenced by the easy handle
		std::string method_;
		std::string url_;
		std::vector<char> request_body_;
		std::vector<std::string> request_headers_;
		std::vector<char> response_body_;
		std::vector<std::pair<std::u16string, std::u16string>> response_headers_;
		std::vector<UHR_Header> response_headers_storage_;

		std::size_t request_body_cursor_ = 0;
		std::uint32_t http_status_ = 0;
		bool completed_ = false;
		bool cancelled_ = false;
	};

} // namespace uhr
