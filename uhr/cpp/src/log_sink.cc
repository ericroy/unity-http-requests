#include "log_sink.h"

namespace uhr {

	LogSink::Acquired::~Acquired() {
		if (sink_->callback_ != nullptr) {
			auto s = oss_.str();
			sink_->callback_(s.c_str(), static_cast<std::uint32_t>(s.size()), sink_->user_data_);
		}
	}

	LogSink::Acquired::Acquired(LogSink *sink) : sink_(sink), lock_(sink->mutex_) {}

	LogSink::Acquired LogSink::Acquire() {
		return Acquired(this);
	}

	void LogSink::Set(UHR_LoggingCallback callback, void* user_data) {
		std::lock_guard<std::mutex> lock(mutex_);
		user_data_ = user_data;
		callback_ = callback;
	}

} // namespace uhr
