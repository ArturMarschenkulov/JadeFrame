#pragma once
#include <unordered_map>
#include <string>

typedef unsigned long DWORD;

typedef unsigned int     UINT;
typedef unsigned __int64 UINT_PTR;
typedef UINT_PTR         WPARAM;
typedef __int64          LONG_PTR;
typedef LONG_PTR         LPARAM;

namespace JadeFrame {
namespace win32 {

struct EventMessage {
    HWND   hWnd;
    UINT   message;
    WPARAM wParam;
    LPARAM lParam;
};

class EventMessageMap {
public:
    EventMessageMap() noexcept;
    auto operator()(const EventMessage& m) const noexcept -> std::string;

private:
    std::unordered_map<DWORD, std::string> map;
};
} // namespace win32
} // namespace JadeFrame