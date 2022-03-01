#include "util.h"
#include "utf8cpp/utf8.h"

namespace uhr {

	std::u16string ToUTF16(const std::string& s) {
		return utf8::utf8to16(s);
	}

	std::u16string ToUTF16(const char* s) {
		return utf8::utf8to16(std::string_view(s));
	}

	std::u16string ToUTF16(const char *begin_iter, const char *end_iter) {
		return utf8::utf8to16(std::string_view(begin_iter, end_iter - begin_iter));
	}

	std::string ToUTF8(UHR_StringRef sr) {
		return utf8::utf16to8(std::u16string_view(reinterpret_cast<const char16_t *>(sr.characters), sr.length));
	}

	UHR_StringRef ToStringRef(const std::u16string &s) {
		UHR_StringRef sr = {};
		sr.characters = reinterpret_cast<const std::uint16_t *>(s.c_str());
		sr.length = static_cast<std::uint32_t>(s.size());
		return sr;
	}

} // namespace uhr