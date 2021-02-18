#pragma once
#include <sstream>
#include <mutex>
#include "UnityHttpRequests.h"

namespace uhr {

	class LogSink {
	public:
		class Acquired {
			friend class LogSink;
		public:
			Acquired(Acquired &&) = default;
			~Acquired();
			template<typename T> std::ostringstream& operator<<(T val) { oss_ << val; return oss_; }
		private:
			Acquired(LogSink *sink);
			Acquired(const Acquired &) = delete;
			LogSink *sink_;
			std::lock_guard<std::mutex> lock_;
			std::ostringstream oss_;
		};

		Acquired Acquire();
		void Set(UHR_LoggingCallback callback, void* user_data);

	private:
		std::mutex mutex_;
		void* user_data_ = nullptr;
		UHR_LoggingCallback callback_ = nullptr;
	};

} // namespace uhr
