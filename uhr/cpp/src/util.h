#pragma once
#include <string>
#include "UnityHttpRequests.h"

namespace uhr {

    std::u16string ToUTF16(const std::string& s);

    std::string ToUTF8(UHR_StringRef sr);

} // namespace uhr
