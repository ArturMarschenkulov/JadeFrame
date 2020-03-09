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

	std::array<float, 4>& operator[](const int index); // for writing
	const std::array<float, 4>& operator[](const int index) const; // for reading

	Vec3 operator*(const Vec3& vector) const;
	Vec4 operator*(const Vec4& vector) const;
	Mat4 operator*(const Mat4& other) const;

	static Mat4 ortho(float left, float right, float buttom, float top, float near, float far);
	static Mat4 perspective(float fovy, float aspect, float near, float far);

	static Mat4 translate(Mat4 const& mat, const Vec3& trans);
	static Mat4 translate(const Vec3& trans);
	static Mat4 rotate(Mat4 const& mat, float angle, const Vec3& axis);
	static Mat4 rotate(float angle, const Vec3& axis);
	static Mat4 scale(Mat4 const& mat, const Vec3& scale);
	static Mat4 scale(const Vec3& scale);

	static Mat4 lookAt(const Vec3& camera, Vec3 object, Vec3 up) {

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
	
	};

	Mat4& makeEchelon() {
		int lead = 0;
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
		int lead = 0;
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