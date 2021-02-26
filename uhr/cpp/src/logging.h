#pragma once
#include <sstream>
#include <mutex>
#include "UnityHttpRequests.h"

#define uhr_LOG_EXPRESSION( prefix, expr ) ::uhr::g_log_sink.Acquire() << prefix << expr

#define UHR_LOG_DEBUG(...) uhr_LOG_EXPRESSION("UHR[DEBUG]: ", __VA_ARGS__)
#define UHR_LOG_INFO(...) uhr_LOG_EXPRESSION("UHR[INFO]: ", __VA_ARGS__)
#define UHR_LOG_ERROR(...) uhr_LOG_EXPRESSION("UHR[ERROR]: ", __VA_ARGS__)
#define UHR_LOG_CRITICAL(...) uhr_LOG_EXPRESSION("UHR[CRITICAL]: ", __VA_ARGS__)

namespace uhr {

	class LogSink {
	public:
		class Acquired {
			friend class LogSink;
		public:
			Acquired(Acquired &&) = default;
			~Acquired();
			template<typename T> Acquired& operator<<(const T &val) { oss_ << val; return *this; }
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

	extern LogSink g_log_sink;

} // namespace uhr
