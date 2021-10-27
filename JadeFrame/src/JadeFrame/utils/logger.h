#pragma once
#include <format>
#include <iostream>
#include <string>

namespace JadeFrame {
class Logger {
public:
	enum class LEVEL {
		TRACE,
		DEBUG,
		INFO,
		WARN,
		ERR,
		CRITICAL,
		OFF
	};
	template<class ...Types>
	static auto log(const char* text, const Types& ... args) -> void;
	template<class ...Types>
	static auto log(LEVEL level, const char* text, const Types& ... args) -> void;

	template<class ...Types>
	static auto trace(const char* text, const Types& ... args) -> void;
	template<class ...Types>
	static auto debug(const char* text, const Types& ... args) -> void;
	template<class ...Types>
	static auto info(const char* text, const Types& ... args) -> void;
	template<class ...Types>
	static auto warn(const char* text, const Types& ... args) -> void;
	template<class ...Types>
	static auto err(const char* text, const Types& ... args) -> void;
	template<class ...Types>
	static auto critical(const char* text, const Types& ... args) -> void;

};

template<class ...Types>
static auto Logger::log(const char* text, const Types& ... args) -> void {
	std::string s = std::format<Types...>(text, args...);
	std::cout << s << "\n";
}
template<class ...Types>
static auto Logger::log(LEVEL /*level*/, const char* text, const Types& ... args) -> void {
	std::string s = std::format<Types...>(text, args...);
	std::cout << s << "\n";
}





template<class ...Types>
static auto Logger::trace(const char* text, const Types& ... args) -> void {
	Logger::log(LEVEL::TRACE, text, args...);
}
template<class ...Types>
static auto Logger::debug(const char* text, const Types& ... args) -> void {
	Logger::log(LEVEL::DEBUG, text, args...);
}
template<class ...Types>
static auto Logger::info(const char* text, const Types& ... args) -> void {
	Logger::log(LEVEL::INFO, text, args...);
}
template<class ...Types>
static auto Logger::warn(const char* text, const Types& ... args) -> void {
	Logger::log(LEVEL::WARN, text, args...);
}
template<class ...Types>
static auto Logger::err(const char* text, const Types& ... args) -> void {
	Logger::log(LEVEL::ERR, text, args...);
}
template<class ...Types>
static auto Logger::critical(const char* text, const Types& ... args) -> void {
	Logger::log(LEVEL::CRITICAL, text, args...);
}
}