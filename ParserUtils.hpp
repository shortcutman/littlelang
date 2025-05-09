
//------------------------------------------------------------------------------
// ParserUtils.hpp
//------------------------------------------------------------------------------

#pragma once

#include <string_view>
#include <algorithm>

inline void trim_left(std::string_view& view) {
    while (std::isspace(view[0])) {
        view.remove_prefix(1);
    }
}

inline void trim_right(std::string_view& view) {
    while (std::isspace(view[view.size() - 1])) {
        view.remove_suffix(1);
    }
}

inline void trim_sides(std::string_view& view) {
    trim_left(view);
    trim_right(view);
}

inline bool haswhitespace(const std::string_view view) {
    return std::any_of(view.begin(), view.end(), [] (auto c) { return std::isspace(c); });
}

inline size_t find_block_end(std::string_view input) {
    if (input[0] != '{') {
        throw std::runtime_error("unexpected starting condition");
    }

    size_t index = 0;
    size_t requiredCurls = 0;
    for (auto c : input) {
        if (c == '{') {
            requiredCurls++;
        } else if (c == '}') {
            requiredCurls--;
        }

        if (requiredCurls == 0) {
            return index;
        }
        index++;
    }

    return std::string_view::npos;
}