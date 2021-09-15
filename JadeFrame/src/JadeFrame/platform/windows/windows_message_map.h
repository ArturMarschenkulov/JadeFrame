#pragma once
#include <unordered_map>
#include <string>

typedef unsigned long DWORD;

typedef unsigned int        UINT;
typedef unsigned __int64	UINT_PTR;
typedef UINT_PTR            WPARAM;
typedef __int64				LONG_PTR;
typedef LONG_PTR            LPARAM;

namespace JadeFrame {

struct WindowsMessage {
	HWND hWnd;
	UINT message;
	WPARAM wParam;
	LPARAM lParam;
};

class WindowsMessageMap {
public:
	WindowsMessageMap() noexcept;
	auto operator()(const WindowsMessage& m) const noexcept -> std::string;
private:
	std::unordered_map<DWORD, std::string> map;
};
}