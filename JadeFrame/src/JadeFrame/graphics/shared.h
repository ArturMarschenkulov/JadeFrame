#pragma once
#include "JadeFrame/defines.h"
#include <JadeFrame/math/mat_4.h>
//#include "Mesh.h"

namespace JadeFrame {
/*
	TODO: Consider whether this is a good way and whether it is worth it to introdcue inheritance.
		Right now this is mainly used as a sanity check such that all renderers have a common interface.
*/
class Windows_Window;
class Object;
class Color;

class IRenderer {
public:
	virtual auto set_clear_color(const Color& color) -> void = 0;


	virtual auto clear_background() -> void = 0;
	virtual auto submit(const Object& obj) -> void = 0;

	virtual auto present() const -> void = 0;
	virtual auto render(const Matrix4x4& view_projection) -> void = 0;

	virtual auto set_viewport(u32 x, u32 y, u32 width, u32 height) const -> void = 0;
	
	virtual auto take_screenshot(const char* filename) -> void = 0;

	virtual auto main_loop() -> void = 0;
};
}