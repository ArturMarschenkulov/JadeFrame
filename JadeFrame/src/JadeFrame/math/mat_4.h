#pragma once
#include <array>
#include "vec_4.h"
#include "vec_3.h"
#include <vector>

/*
	This matrix is column major

	mat[n][m]
	n = columns
	m = rows

	n0 n1 n2 (rank)
	 __ __ __
m0	|__|__|__|
m1	|__|__|__|
m2	|__|__|__|

*/
class Matrix4x4;
class Matrix4x4 {
public:
	Matrix4x4();
	Matrix4x4(const float diagonal);
	Matrix4x4(const Vec4& col1, const Vec4& col2, const Vec4& col3, const Vec4& col4);
	Matrix4x4(const Matrix4x4& mat);

	auto operator=(const Matrix4x4& mat)->Matrix4x4&;

	auto operator[](const int index)->std::array<float, 4>&; // for writing
	auto operator[](const int index) const -> const std::array<float, 4>&; // for reading

	auto operator*(const Vec4& vector) const->Vec4;
	auto operator*(const Matrix4x4& other) const->Matrix4x4;

public: // static methods for matrices
	static auto orthogonal_projection_matrix(float left, float right, float buttom, float top, float near, float far)->Matrix4x4;
	static auto perspective_projection_matrix(float fovy, float aspect, float near, float far)->Matrix4x4;
	static auto translation_matrix(const Vec3& trans)->Matrix4x4;
	static auto rotation_matrix(float angle, const Vec3& axis) ->Matrix4x4;
	static auto scale_matrix(const Vec3& scale) -> Matrix4x4;
	static auto shear_matrix()->Matrix4x4;
	static auto look_at_matrix(const Vec3& camera, Vec3 object, Vec3 up)->Matrix4x4;

public:
	auto get_determinant() const -> float;
	auto get_echelon() const->Matrix4x4;
	auto get_transpose() const->Matrix4x4;
	auto get_inverted() const->Matrix4x4;

	auto is_invertible() const -> bool;
	auto get_rank() const -> int;

	auto make_echelon()->Matrix4x4&;


private:
	union {
		std::array<std::array<float, 4>, 4> el;
		std::array<float, 4> colVec;
	};
};

//auto print_m(const Matrix4x4& mat) -> void {
//	for (int row = 0; row < 4; row++) {
//		for (int col = 0; col < 4; col++) {
//			std::cout << "\t" << mat[col][row] << " ";
//		}
//		std::cout << std::endl;
//	}
//}
//auto to_string(const Matrix4x4& mat) -> std::string {
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