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

	Mat4& makeEchelon() {
		int lead = 0;
		int colCount = 4;
		int rowCount = 4;
		// go through every column
		for (int col = 0; col < colCount; col++) {
			for (int row = col + 1; row < rowCount; row++) {
				if (el[col][row] != 0) {
					float factor = el[col][row] / el[col][col];
					for (int col2 = 0; col2 < colCount; col2++) {
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
		for (int col = 0; col < colCount; col++) {
			for (int row = col + 1; row < rowCount; row++) {
				if (m[col][row] != 0) {
					float factor = m[col][row] / m[col][col];
					for (int col2 = 0; col2 < colCount; col2++) {
						m[col2][row] -= factor * m[col2][col];
					}
				}
			}
		}
		return m;
	}

	void printM() {
		for (int row = 0; row < 4; row++) {
			for (int col = 0; col < 4; col++) {
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