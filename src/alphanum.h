#pragma once
#include <string_view>

// reimplementation of alphanum comparison algorithm by Dave Koelle
// built for modern C++ with optional case-insensitive checking and wide string support

namespace doj
{
    enum StringComparison { CASE_SENSITIVE, CASE_INSENSITIVE };

    int alphanum_comp(std::string_view l, std::string_view r, StringComparison comp = CASE_SENSITIVE);
    int alphanum_comp(std::wstring_view l, std::wstring_view r, StringComparison comp = CASE_SENSITIVE);

    template<typename T, StringComparison Comp = CASE_SENSITIVE, typename Enable = void>
    struct alphanum_less;

    template<typename T, StringComparison Comp>
    struct alphanum_less<T, Comp, std::enable_if_t<std::is_convertible_v<T, std::string_view>>>
    {
        bool operator()(std::string_view left, std::string_view right) const
        {
            return alphanum_comp(left, right, Comp) < 0;
        }
    };

    template<typename T, StringComparison Comp>
    struct alphanum_less<T, Comp, std::enable_if_t<std::is_convertible_v<T, std::wstring_view>>>
    {
        bool operator()(std::wstring_view left, std::wstring_view right) const
        {
            return alphanum_comp(left, right, Comp) < 0;
        }
    };
}
