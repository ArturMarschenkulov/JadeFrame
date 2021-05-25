#include "utils.h"
#include <cstdlib> 
#include <ctime>



#if 0
#include <string>
#include <codecvt>
#include <locale>
#include <string>
using convert_t = std::codecvt_utf8<wchar_t>;
static std::wstring_convert<convert_t, wchar_t> strconverter;

static auto from_wstring_to_string(std::wstring wstr) -> std::string {
	return strconverter.to_bytes(wstr);
}

static auto from_string_to_wstring(std::string str) -> std::wstring {
	return strconverter.from_bytes(str);
}
#elif _WIN32

#include <Windows.h>
// Convert a wide Unicode string to an UTF8 string
auto from_wstring_to_string(const std::wstring& wstr) -> std::string {
	if (wstr.empty()) return std::string();
	i32 size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (i32)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (i32)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}
// Convert an UTF8 string to a wide Unicode String
auto from_string_to_wstring(const std::string& str) -> std::wstring {
	if (str.empty()) return std::wstring();
	i32 size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (i32)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (i32)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}
#endif
auto custom_simple_hash_0(const std::string& str) -> u32 {
	u32 hash = 0;
	for (auto& it : str) {
		hash = 37 * hash + 17 * static_cast<char>(it);
	}
	return hash;
}
static bool is_srand = false;
auto get_random_number(i32 begin, i32 end)-> i32  {
	if (is_srand == false) {
		srand((u32)time(0));
		is_srand = true;
	}
	return (rand() % end) + begin;
}

auto map_range(const f64 x, const f64 in_min, const f64 in_max, const f64 out_min, const f64 out_max) -> f64 {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
