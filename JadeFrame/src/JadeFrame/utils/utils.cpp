#include "utils.h"
#include "pch.h"
#include <cstdlib>
#include <ctime>

namespace JadeFrame {

#if _WIN32
#include <Windows.h>
auto from_wstring_to_string(const std::wstring& wstr) -> std::string {
    if (wstr.empty()) { return std::string(); }
    i32 size_needed =
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (i32)wstr.size(), NULL, 0, NULL, NULL);
    std::string str_to(size_needed, 0);
    WideCharToMultiByte(
        CP_UTF8, 0, &wstr[0], (i32)wstr.size(), &str_to[0], size_needed, NULL, NULL);
    return str_to;
}
auto from_string_to_wstring(const std::string& str) -> std::wstring {
    if (str.empty()) { return std::wstring(); }
    i32          size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (i32)str.size(), NULL, 0);
    std::wstring wstr_to(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (i32)str.size(), &wstr_to[0], size_needed);
    return wstr_to;
}
#endif

auto custom_simple_hash_0(const std::string& str) -> u32 {
    u32 hash = 0;
    for (auto& it : str) { hash = 37 * hash + 17 * static_cast<char>(it); }
    return hash;
}
static bool is_srand = false;
auto        get_random_number(i32 begin, i32 end) -> i32 {
    if (is_srand == false) {
        srand(static_cast<u32>(time(0)));
        is_srand = true;
    }
    return (rand() % end) + begin;
}

auto map_range(
    const f64 x, const f64 in_min, const f64 in_max, const f64 out_min, const f64 out_max) -> f64 {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

} // namespace JadeFrame