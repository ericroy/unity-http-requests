#include "util.h"
#include <locale>
#include <codecvt>

namespace uhr {

	std::u16string ToUTF16(const std::string& s) {
		// Cannot qualify char16_t with std:: namespace here, weird.
		std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
		return convert.from_bytes(s);
	}

	std::string ToUTF8(UHR_StringRef sr) {
		// Cannot qualify char16_t with std:: namespace here, weird.
		std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert; 
		return convert.to_bytes(
			reinterpret_cast<const char16_t *>(sr.characters),
			reinterpret_cast<const char16_t *>(sr.characters) + sr.length
		);
	}
		
} // namespace uhr