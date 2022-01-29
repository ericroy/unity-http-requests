#pragma once
#include <string>
#include "UnityHttpRequests.h"

namespace uhr {

	std::u16string ToUTF16(const std::string& s);

	std::u16string ToUTF16(const char* s);

	std::u16string ToUTF16(const char *begin_iter, const char *end_iter);

	std::string ToUTF8(UHR_StringRef sr);

	UHR_StringRef ToStringRef(const std::u16string &s);

} // namespace uhr
