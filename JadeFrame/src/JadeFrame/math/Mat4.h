#pragma once
#include <array>
#include "Vec4.h"
#include "Vec3.h"
#include <vector>
#include <iostream>


class Mat4 {
public:
	Mat4();
	Mat4(float diagonal);
	Mat4(Vec4 col1, Vec4 col2, Vec4 col3, Vec4 col4);

	auto operator[](const int index) -> std::array<float, 4>&; // for writing
	auto operator[](const int index) const -> const std::array<float, 4>&; // for reading

	auto operator*(const Vec3& vector) const -> Vec3;
	auto operator*(const Vec4& vector) const -> Vec4;
	auto operator*(const Mat4& other) const -> Mat4;

	static auto ortho(float left, float right, float buttom, float top, float near, float far) -> Mat4;
	static auto perspective(float fovy, float aspect, float near, float far) -> Mat4;

	static auto translate(Mat4 const& mat, const Vec3& trans) -> Mat4;
	static auto translate(const Vec3& trans) -> Mat4;
	static auto rotate(Mat4 const& mat, float angle, const Vec3& axis) -> Mat4;
	static auto rotate(float angle, const Vec3& axis) -> Mat4;
	static auto scale(Mat4 const& mat, const Vec3& scale) -> Mat4;
	static auto scale(const Vec3& scale) -> Mat4;

	static auto lookAt(const Vec3& camera, Vec3 object, Vec3 up) -> Mat4 {

#if 1
		Vec3 forward = (object - camera).normalize();
		Vec3 left = up.cross(forward).normalize();
		Vec3 upp = forward.cross(left);

		Mat4 matrix;
		matrix[0][0] = left.x;
		matrix[0][1] = left.y;
		matrix[0][2] = left.z;

		matrix[1][0] = -upp.x;
		matrix[1][1] = -upp.y;
		matrix[1][2] = -upp.z;

		matrix[2][0] = -forward.x;
		matrix[2][1] = -forward.y;
		matrix[2][2] = -forward.z;

		matrix[3][0] = (-left.x * camera.x - left.y * camera.y - left.z * camera.z);
		matrix[3][1] = -(-upp.x * camera.x - upp.y * camera.y - up.z * camera.z);
		matrix[3][2] = -(-forward.x * camera.x - forward.y * camera.y - forward.z * camera.z);
		return matrix;
#else 
		Mat4 result;
		Vec3 f = (object - camera).normalize();
		Vec3 s = f.cross(up.normalize());
		Vec3 u = s.cross(f);

		result[0][0] = s.x;
		result[0][1] = s.y;
		result[0][2] = s.z;

		result[1][0] = u.x;
		result[1][1] = u.y;
		result[1][2] = u.z;

		result[2][0] = -f.x;
		result[2][1] = -f.y;
		result[2][2] = -f.z;

		return result * Mat4::translate(Vec3(-camera.x, -camera.y, -camera.z));
#endif
	
	}

	auto makeEchelon() -> Mat4& {
		int colCount = 4;
		int rowCount = 4;
		// go through every column
		for(int col = 0; col < colCount; col++) {
			for(int row = col + 1; row < rowCount; row++) {
				if(el[col][row] != 0) {
					float factor = el[col][row] / el[col][col];
					for(int col2 = 0; col2 < colCount; col2++) {
						el[col2][row] -= factor * el[col2][col];
					}
				}
			}
		}
		return *this;
	}
	Mat4 getEchelon() {
		Mat4 m = *this;
		int colCount = 4;
		int rowCount = 4;
		// go through every column
		for(int col = 0; col < colCount; col++) {
			for(int row = col + 1; row < rowCount; row++) {
				if(m[col][row] != 0) {
					float factor = m[col][row] / m[col][col];
					for(int col2 = 0; col2 < colCount; col2++) {
						m[col2][row] -= factor * m[col2][col];
					}
				}
			}
		}
		return m;
	}

	void printM() {
		for(int row = 0; row < 4; row++) {
			for(int col = 0; col < 4; col++) {
				std::cout << "\t" << el[col][row] << " ";
			}
			std::cout << std::endl;
		}
	}
private:
	union {
		std::array<std::array<float, 4>, 4> el;
		std::array<float, 4> colVec;
	};


	//std::array<std::array<float, 4>, 4> el;
};