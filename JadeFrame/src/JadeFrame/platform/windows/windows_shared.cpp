#include "windows_shared.h"
// #include "JadeFrame/src/JadeFrame/prelude.h"
#include <Windows.h>
#include <string>
namespace JadeFrame {
namespace platform {
namespace win32 {
#if 1 // TODO: Utility function, move it to another place

auto to_wide_char(const char* char_array) -> wchar_t* {
    wchar_t* wString = new wchar_t[4096];
    // wchar_t wString[4096];
    ::MultiByteToWideChar(CP_ACP, 0, char_array, -1, wString, 4096);
    return wString;
}
auto to_multi_byte(const wchar_t* wide_char_array) -> char* {
    char* cString = new char[4096];
    // char cString[4096];
    ::WideCharToMultiByte(CP_ACP, 0, wide_char_array, -1, cString, 4096, NULL, NULL);
    return cString;
}
auto to_wide_char(const std::string& string) -> std::wstring {
    std::wstring wString;
    wString.resize(string.size());
    ::MultiByteToWideChar(CP_ACP, 0, string.c_str(), -1, &wString[0], string.size());
    return wString;
}
auto to_multi_byte(const std::wstring& wstring) -> std::string {
    std::string cString;
    cString.resize(wstring.size());
    ::WideCharToMultiByte(CP_ACP, 0, wstring.c_str(), -1, &cString[0], wstring.size(), NULL, NULL);
    return cString;
}

auto from_wstring_to_string(const std::wstring& wstr) -> std::string {
    if (wstr.empty()) { return std::string(); }
    i32         size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (i32)wstr.size(), NULL, 0, NULL, NULL);
    std::string str_to(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (i32)wstr.size(), &str_to[0], size_needed, NULL, NULL);
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
} // namespace win32
} // namespace platform
} // namespace JadeFrame