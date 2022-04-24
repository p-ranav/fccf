#pragma once
#include <cstddef>
#include <string_view>

#if defined(__SSE2__)

namespace search
{
size_t sse2_strstr_v2(const std::string_view& s,
                      const std::string_view& needle);

}  // namespace search

#endif
