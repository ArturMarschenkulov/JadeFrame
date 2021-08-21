#pragma once
#include <format>
#include <iostream>
#include <string>

namespace JadeFrame {
class Logger {
public:
	//enum class TYPE {
	//
	//};
	template<class ...Types>
	static auto log(const char* text, const Types& ... args) -> void;
};

template<class ...Types>
static auto Logger::log(const char* text, const Types& ... args) -> void {
	std::string s = std::format<Types...>(text, args...	);
	std::cout << s << "\n";
}
}