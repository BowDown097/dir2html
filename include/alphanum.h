#pragma once
#include <charconv>
#include <cctype>
#include <cwctype>
#include <string_view>

// reimplementation of alphanum comparison algorithm by Dave Koelle
// built for modern C++ with optional case-insensitive checking and wide string support

namespace doj
{
    enum StringComparison { CASE_SENSITIVE, CASE_INSENSITIVE };

    int alphanum_comp(std::string_view l, std::string_view r, StringComparison comp = CASE_SENSITIVE)
    {
        enum mode_t { STRING, NUMBER } mode = STRING;

        auto lit = l.begin(), rit = r.begin();
        while (lit != l.end() && rit != r.end())
        {
            if (mode == STRING)
            {
                while (lit != l.end() && rit != r.end())
                {
                    unsigned char l_char = *lit;
                    unsigned char r_char = *rit;
                    // check if chars are digits
                    const bool l_digit = isdigit(l_char);
                    const bool r_digit = isdigit(r_char);
                    // if both are digits, continue in NUMBER mode
                    if (l_digit && r_digit)
                    {
                        mode = NUMBER;
                        break;
                    }
                    // if only the left is a digit, compared string is lower
                    if (l_digit) return -1;
                    // if only the right is a digit, compared string is greater
                    if (r_digit) return +1;
                    // compute the difference, return said difference if they differ
                    const int diff = comp == CASE_SENSITIVE
                        ? l_char - r_char : tolower(l_char) - tolower(r_char);
                    if (diff != 0) return diff;
                    // otherwise process the next characters
                    ++lit;
                    ++rit;
                }
            }
            else // mode == NUMBER
            {
                // get the left number
                unsigned long l_int{};
                auto l_result = std::from_chars(&*lit, l.data() + l.size(), l_int);
                lit += l_result.ptr - &*lit;
                // get the right number
                unsigned long r_int{};
                auto r_result = std::from_chars(&*rit, r.data() + r.size(), r_int);
                rit += r_result.ptr - &*rit;
                // compare the difference, return said difference if they differ
                const long diff = l_int - r_int;
                if (diff != 0)
                    return diff;
                // otherwise process the next substring in STRING mode
                mode = STRING;
            }
        }

        if (rit != r.end()) return -1;
        if (lit != l.end()) return +1;
        return 0;
    }

    int alphanum_comp(std::wstring_view l, std::wstring_view r, StringComparison comp = CASE_SENSITIVE)
    {
        enum mode_t { STRING, NUMBER } mode = STRING;

        auto lit = l.begin(), rit = r.begin();
        while (lit != l.end() && rit != r.end())
        {
            if (mode == STRING)
            {
                while (lit != l.end() && rit != r.end())
                {
                    wchar_t l_char = *lit;
                    wchar_t r_char = *rit;
                    // check if chars are digits
                    const bool l_digit = iswdigit(l_char);
                    const bool r_digit = iswdigit(r_char);
                    // if both are digits, continue in NUMBER mode
                    if (l_digit && r_digit)
                    {
                        mode = NUMBER;
                        break;
                    }
                    // if only the left is a digit, compared string is lower
                    if (l_digit) return -1;
                    // if only the right is a digit, compared string is greater
                    if (r_digit) return +1;
                    // compute the difference, return said difference if they differ
                    const int diff = comp == CASE_SENSITIVE
                        ? l_char - r_char : towlower(l_char) - towlower(r_char);
                    if (diff != 0) return diff;
                    // otherwise process the next characters
                    ++lit;
                    ++rit;
                }
            }
            else // mode == NUMBER
            {
                // get the left number
                wchar_t* end;
                unsigned long l_int = wcstoul(&*lit, &end, 10);
                lit += end - &*lit;
                // get the right number
                unsigned long r_int = wcstoul(&*rit, &end, 10);
                rit += end - &*rit;
                // compare the difference, return said difference if they differ
                const long diff = l_int - r_int;
                if (diff != 0)
                    return diff;
                // otherwise process the next substring in STRING mode
                mode = STRING;
            }
        }

        if (rit != r.end()) return -1;
        if (lit != l.end()) return +1;
        return 0;
    }

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
