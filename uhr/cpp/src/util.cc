#include "util.h"
#include <locale>
#include <codecvt>

namespace uhr {

	std::u16string ToUTF16(const std::string& s) {
		std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
		return convert.from_bytes(s);
	}

	std::u16string ToUTF16(const char *begin_iter, const char *end_iter) {
		std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
		return convert.from_bytes(begin_iter, end_iter);
	}

	std::string ToUTF8(UHR_StringRef sr) {
		std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert; 
		return convert.to_bytes(
			reinterpret_cast<const char16_t *>(sr.characters),
			reinterpret_cast<const char16_t *>(sr.characters) + sr.length
		);
	}

	UHR_StringRef ToStringRef(const std::u16string &s) {
		UHR_StringRef sr = {};
		sr.characters = reinterpret_cast<const std::uint16_t *>(s.c_str());
		sr.length = static_cast<std::uint32_t>(s.size());
		return sr;
	}

} // namespace uhr