#pragma once
#include "JadeFrame/prelude.h"
namespace JadeFrame {
// namespace platform {
namespace win32 {
// auto convert_char_array_to_LPCWSTR(const char* charArray) -> wchar_t*;
auto to_wide_char(const char* char_array) -> wchar_t*;
auto to_multi_byte(const wchar_t* wide_char_array) -> char*;
auto from_wstring_to_string(const std::wstring& wstr) -> std::string;
auto from_string_to_wstring(const std::string& str) -> std::wstring;
} // namespace win32
// } // namespace platform
} // namespace JadeFrame