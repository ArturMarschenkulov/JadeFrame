#pragma once
#include <array>
#include "vec_4.h"
#include "vec_3.h"
#include <vector>

/*
	This matrix is column major?

	mat[n][m]
	n = columns
	m = rows

	n0 n1 n2
	 __ __ __
m0	|__|__|__|
m1	|__|__|__|
m2	|__|__|__|

*/

class Mat4 {
public:
	Mat4();
	Mat4(const float diagonal);
	Mat4(const Vec4& col1, const Vec4& col2, const Vec4& col3, const Vec4& col4);

	auto operator[](const int index)->std::array<float, 4>&; // for writing
	auto operator[](const int index) const -> const std::array<float, 4>&; // for reading

	//auto operator*(const Vec3& vector) const -> Vec3;
	auto operator*(const Vec4& vector) const->Vec4;
	auto operator*(const Mat4& other) const->Mat4;

	static auto ortho(float left, float right, float buttom, float top, float near, float far)->Mat4;
	static auto perspective(float fovy, float aspect, float near, float far)->Mat4;

	static auto translate(Mat4 const& mat, const Vec3& trans)->Mat4;
	static auto translate(const Vec3& trans)->Mat4;
	static auto rotate(Mat4 const& mat, float angle, const Vec3& axis)->Mat4;
	static auto rotate(float angle, const Vec3& axis)->Mat4;
	static auto scale(Mat4 const& mat, const Vec3& scale)->Mat4;
	static auto scale(const Vec3& scale)->Mat4;

	static auto look_at(const Vec3& camera, Vec3 object, Vec3 up) -> Mat4 {
		auto look_at_LH = [](const Vec3& eye, const Vec3& center, const Vec3& up) {
			Vec3 const f((center - eye).get_normal());
			Vec3 const s(up.cross(f).get_normal());
			Vec3 const u(f.cross(s));

			Mat4 Result(1);
			Result[0][0] = s.x;
			Result[1][0] = s.y;
			Result[2][0] = s.z;

			Result[0][1] = u.x;
			Result[1][1] = u.y;
			Result[2][1] = u.z;

			Result[0][2] = f.x;
			Result[1][2] = f.y;
			Result[2][2] = f.z;

			Result[3][0] = -s.dot(eye);
			Result[3][1] = -u.dot(eye);
			Result[3][2] = -s.dot(eye);
			return Result;
		};
		auto look_at_RH = [](const Vec3& eye, const Vec3& center, const Vec3& up) {
			Vec3 const f((center - eye).get_normal());
			Vec3 const s(f.cross(up).get_normal());
			Vec3 const u(s.cross(f));

			Mat4 Result(1);
			Result[0][0] = s.x;
			Result[1][0] = s.y;
			Result[2][0] = s.z;

			Result[0][1] = u.x;
			Result[1][1] = u.y;
			Result[2][1] = u.z;

			Result[0][2] = -f.x;
			Result[1][2] = -f.y;
			Result[2][2] = -f.z;

			Result[3][0] = -s.dot(eye);
			Result[3][1] = -u.dot(eye);
			Result[3][2] = f.dot(eye);
			return Result;
		};
		return look_at_RH(camera, object, up);
	#if 1
		Vec3 forward = (object - camera).get_normal();
		Vec3 left = up.cross(forward).get_normal();
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

	auto get_determinant() const -> float;
	auto make_echelon() -> Mat4& {
		int col_count = 4;
		int row_count = 4;
		// go through every column
		for (int col = 0; col < col_count; col++) {
			for (int row = col + 1; row < row_count; row++) {
				if (el[col][row] != 0) {
					float factor = el[col][row] / el[col][col];
					for (int col2 = 0; col2 < col_count; col2++) {
						el[col2][row] -= factor * el[col2][col];
					}
				}
			}
		}
		return *this;
	}
	auto get_echelon() const -> Mat4 {
		Mat4 m = *this;
		int col_count = 4;
		int row_count = 4;
		// go through every column
		for (int col = 0; col < col_count; col++) {
			for (int row = col + 1; row < row_count; row++) {
				if (m[col][row] != 0) {
					float factor = m[col][row] / m[col][col];
					for (int col2 = 0; col2 < col_count; col2++) {
						m[col2][row] -= factor * m[col2][col];
					}
				}
			}
		}
		return m;
	}


private:
	union {
		std::array<std::array<float, 4>, 4> el;
		std::array<float, 4> colVec;
	};
	//std::array<std::array<float, 4>, 4> el;
	};

//auto print_m(const Mat4& mat) -> void {
//	for (int row = 0; row < 4; row++) {
//		for (int col = 0; col < 4; col++) {
//			std::cout << "\t" << mat[col][row] << " ";
//		}
//		std::cout << std::endl;
//	}
//}
//auto to_string(const Mat4& mat) -> std::string {
//	std::string result;
//	for (int row = 0; row < 4; row++) {
//		for (int col = 0; col < 4; col++) {
//			//result += "\t";
//			result += std::to_string(mat[col][row]);
//			result += " ";
//		}
//		result += "\n";
//	}
//
//	return result;
//}