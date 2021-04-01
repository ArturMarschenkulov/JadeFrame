#pragma once
#include <unordered_map>
#include <string>

typedef unsigned long DWORD;

struct WindowsMessage;
class WindowsMessageMap {
public:
	WindowsMessageMap() noexcept;
	auto operator()(const WindowsMessage& m) const noexcept -> std::string;
private:
	std::unordered_map<DWORD, std::string> map;
};