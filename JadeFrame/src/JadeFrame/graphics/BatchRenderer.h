#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "BatchShader.h"

#include <vector>
#include "../math/Vec3.h"
#include "../math/Vec2.h"
#include "../math/Mat4.h"
#include "Mesh.h"
#include <stack>

class Camera {
public:
	Camera();
};
class TimeManager {
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

class BatchRenderer {
public:
	BatchRenderer();
	void init(BatchShader* shader);
	void start();
	void handleMesh(Mesh& mesh);
	void end();

private:
	struct BufferData {
		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;

		GLuint VBO = 0;
		GLuint VAO = 0;
		GLuint IBO = 0;

		GLuint vertexOffset = 0;
		GLuint indexOffset = 0;

		GLuint vertexCount = 0;
		GLuint indexCount = 0;

		Color currentColor = { 0.5f, 0.5f, 0.5f, 1.0f };

		void init();
		void add(Mesh& mesh);
		void update();
		void draw();
		void resetCounters();
	};
	BufferData bufferData;
	BatchShader* currentShader = nullptr;

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

	void pushMatrix() { matrixStack.push(); }
	void popMatrix() { matrixStack.pop(); }
private:
	struct MatrixStack {
		std::stack<Mat4> stack;
		Mat4 modelMatrix;
		Mat4 viewMatrix;
		Mat4 projectionMatrix;
		Mat4* currentMatrix;
		Mat4 transformMatrix;
		bool useTransformMatrix;
	public:
		void push() {
			useTransformMatrix = true;
			currentMatrix = &transformMatrix;
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
				useTransformMatrix = false;
			}
		}
	};
	MatrixStack matrixStack;


public:
	Camera camera;
	TimeManager timeManager;
};