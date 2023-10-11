#pragma once
#include "JadeFrame/prelude.h"

struct D3D11_Context {};
struct HWND__;
typedef HWND__* HWND;

class D3D11_Renderer {
public:
    auto set_context(const HWND& window) -> void;

private:
    D3D11_Context m_context;
};