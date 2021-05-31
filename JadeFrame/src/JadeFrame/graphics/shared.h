#pragma once
#include <JadeFrame/math/mat_4.h>


/*
	TODO: Consider whether this is a good way and whether it is worth it to introdcue inheritance.
		Right now this is mainly used as a sanity check such that all renderers have a common interface.
*/
struct HWND__;	typedef HWND__* HWND;
class IRenderer {
	virtual auto swap_buffer(HWND window_handle) const -> void = 0;
	virtual auto clear_background() const -> void = 0;
	virtual auto render_pushed(const Matrix4x4& view_projection) const -> void = 0;
};