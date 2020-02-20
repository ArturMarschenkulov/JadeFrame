#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "BaseShader.h"

#include <vector>
#include "../math/Vec3.h"
#include "../math/Vec2.h"
#include "../math/Mat4.h"
#include "Mesh.h"
#include <stack>

class Camera2 {
public:
	Camera2();
};
class TimeManager2 {
public:
	void handleTime();
private:
	double currentTime = 0.0;
	double previousTime = 0.0;
	double drawTime = 0.0;
	double frameTime = 0.0;
	double updateTime = 0.0;
	double targetTime = 0.0;
};

class BaseRenderer {
public:
	BaseRenderer();
	void init(BaseShader* shader);
	void start();
	void handleMesh(Mesh& mesh);
	void end();

private:
	BaseShader* currentShader = nullptr;

	//Drawing API
public:
	void setColor(const Color& color);
	void setClearColor(const Color& color);
public:
	void updateMatrices();
	void ortho(float left, float right, float buttom, float top, float zNear, float zFar);
	void perspective(float fovy, float aspect, float zNear, float zFar);
	void translate(float x, float y, float z);
	void rotate(float angle, float x, float y, float z);
	void scale(float x, float y, float z);
private:
	struct MatrixStack {
		std::stack<Mat4> stack;
		Mat4 modelMatrix;
		Mat4 viewMatrix;
		Mat4 projectionMatrix;
		Mat4* currentMatrix;
	public:
		void push() {
			stack.push(*currentMatrix);
		}
		void pop() {
			if(!stack.empty()) {
				Mat4 mat = stack.top();
				*currentMatrix = mat;
				stack.pop();
			}
			if(stack.empty()) {
				currentMatrix = &viewMatrix;
			}
		}
	private:

	};
	MatrixStack matrixStack;
	void pushMatrix() { matrixStack.push(); }
	void popMatrix() { matrixStack.pop(); }


public:
	Camera2 camera;
	TimeManager2 timeManager;
};