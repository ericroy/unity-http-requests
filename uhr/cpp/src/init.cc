#include "init.h"
#include <locale>
#include <codecvt>
#include <atomic>
#include <curl/curl.h>

namespace uhr {

	namespace {
		std::atomic_int g_init_count = 0;
	}
	
	void Init() {
		if (std::atomic_fetch_add_explicit(&g_init_count, 1, std::memory_order_relaxed) == 0) {
			curl_global_init(CURL_GLOBAL_DEFAULT);
		}
	}

	void Deinit() {
		if (std::atomic_fetch_sub_explicit(&g_init_count, 1, std::memory_order_relaxed) == 1) {
			curl_global_cleanup();
		}
	}

} // namespace uhr