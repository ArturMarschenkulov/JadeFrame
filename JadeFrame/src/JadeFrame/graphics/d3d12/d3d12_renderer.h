#pragma once
#include "JadeFrame/prelude.h"

struct D3D12_Context {};
struct HWND__;
typedef HWND__* HWND;

class D3D12_Renderer {
public:
    auto set_context(const HWND& window) -> void;

private:
    D3D12_Context m_context;
};