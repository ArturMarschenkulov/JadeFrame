#pragma once
#include "JadeFrame/defines.h"


struct Software_Context {

};
struct HWND__;	typedef HWND__* HWND;
class Software_Renderer {
public:
	auto set_context(const HWND& window) -> void;
private:
	Software_Context m_context;
};