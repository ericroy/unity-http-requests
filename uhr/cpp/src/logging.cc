#include "logging.h"
#include "util.h"

namespace uhr {

	LogSink::Acquired::~Acquired() {
		if (sink_->callback_ != nullptr) {
			auto s = ToUTF16(oss_.str());
			auto ref = ToStringRef(s);
			sink_->callback_(ref, sink_->user_data_);
		}
	}

	LogSink::Acquired::Acquired(LogSink *sink) : sink_(sink), lock_(sink->mutex_) {}

	LogSink::Acquired LogSink::Acquire() {
		return {this};
	}

	void LogSink::Set(UHR_LoggingCallback callback, void* user_data) {
		std::lock_guard<std::mutex> lock(mutex_);
		user_data_ = user_data;
		callback_ = callback;
	}

	// Not inside anonymous namespace, so it can be accessed by the clue
	// logging macros in other code units.
	LogSink g_log_sink;

} // namespace uhr
