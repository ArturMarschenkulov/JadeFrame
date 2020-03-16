#pragma once

#include "Shader.h"

#include <vector>
#include "../math/Vec3.h"
#include "../math/Vec2.h"
#include "../math/Mat4.h"
#include "Mesh.h"
#include <stack>

class BaseRenderer {
public:
	BaseRenderer();
	void init(Shader* shader);
	void start();
	void handle_mesh(Mesh& mesh);
	void end();

private:
	Shader* current_shader = nullptr;

	//Matrix operations
public:
	void update_matrices();
	void ortho(float left, float right, float buttom, float top, float zNear, float zFar);
	void perspective(float fovy, float aspect, float zNear, float zFar);
	void translate(float x, float y, float z);
	void rotate(float angle, float x, float y, float z);
	void scale(float x, float y, float z);

	void push_matrix() { matrix_stack.push(); }
	void pop_matrix() { matrix_stack.pop(); }
private:
	struct MatrixStack {
		std::stack<Mat4> stack;
		Mat4 model_matrix;
		Mat4 view_matrix;
		Mat4 projection_matrix;
		Mat4* current_matrix;
	public:
		void push() {
			stack.push(*current_matrix);
		}
		void pop() {
			if(!stack.empty()) {
				Mat4 mat = stack.top();
				*current_matrix = mat;
				stack.pop();
			}
			if(stack.empty()) {
				current_matrix = &view_matrix;
			}
		}
	private:

	};
	MatrixStack matrix_stack;

	//Drawing API
public:
	void set_color(const Color& color);
	void set_clear_color(const Color& color);
	int num_draw_calls;
};

class JadeFrame {
public:
	//static bool RegisterClass(const HINSTANCE HInstance, const HICON HIcon) {
	//	WNDCLASS wc;
	//	FMemory::Memzero(&wc, sizeof(wc));
	//	wc.style = CS_DBLCLKS; // We want to receive double clicks
	//	wc.lpfnWndProc = AppWndProc;
	//	wc.cbClsExtra = 0;
	//	wc.cbWndExtra = 0;
	//	wc.hInstance = HInstance;
	//	wc.hIcon = HIcon;
	//	wc.hCursor = NULL;	// We manage the cursor ourselves
	//	wc.hbrBackground = NULL; // Transparent
	//	wc.lpszMenuName = NULL;
	//	wc.lpszClassName = FWindowsWindow::AppWindowClass;

	//	if(!::RegisterClass(&wc)) {
	//		//ShowLastError();

	//		// @todo Slate: Error message should be localized!
	//		FSlowHeartBeatScope SuspendHeartBeat;
	//		MessageBox(NULL, TEXT("Window Registration Failed!"), TEXT("Error!"), MB_ICONEXCLAMATION | MB_OK);

	//		return false;
	//	}

	//	return true;
	//}
};