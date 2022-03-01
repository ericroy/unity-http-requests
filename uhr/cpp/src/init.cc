#include "init.h"
#include <mutex>
#include <curl/curl.h>

namespace uhr {

	namespace {
		std::mutex g_init_mutext;
		bool g_initialized = 0;
	}
	
	void Init() {
		std::lock_guard<std::mutex> lock(g_init_mutext);
		if (!g_initialized) {
			curl_global_init(CURL_GLOBAL_DEFAULT);
			g_initialized = true;
		}
	}

	void Deinit() {
		std::lock_guard<std::mutex> lock(g_init_mutext);
		if (g_initialized) {
			curl_global_cleanup();
			g_initialized = false;
		}
	}

} // namespace uhr