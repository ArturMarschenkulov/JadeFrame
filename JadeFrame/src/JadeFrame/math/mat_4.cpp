#include "mat_4.h"

Mat4::Mat4()
	:el(){
	for(int col = 0; col < 4; col++) {
		for(int row = 0; row < 4; row++) {
			el[col][row] = (col == row) ? float(1.0f) : float(0.0f);
		}
	}
	//el[0][0] = 1.0f; el[1][0] = 0.0f; el[2][0] = 0.0f; el[3][0] = 0.0f;
	//el[0][1] = 0.0f; el[1][1] = 1.0f; el[2][1] = 0.0f; el[3][1] = 0.0f;
	//el[0][2] = 0.0f; el[1][2] = 0.0f; el[2][2] = 1.0f; el[3][2] = 0.0f;
	//el[0][3] = 0.0f; el[1][3] = 0.0f; el[2][3] = 0.0f; el[3][3] = 1.0f;
}
Mat4::Mat4(const float digo) {
	for(int col = 0; col < 4; col++) {
		for(int row = 0; row < 4; row++) {
			el[col][row] = (col == row) ? float(digo) : float(0.0f);
		}
	}
	//el[0][0] = digo; el[1][0] = 0.0f; el[2][0] = 0.0f; el[3][0] = 0.0f;
	//el[0][1] = 0.0f; el[1][1] = digo; el[2][1] = 0.0f; el[3][1] = 0.0f;
	//el[0][2] = 0.0f; el[1][2] = 0.0f; el[2][2] = digo; el[3][2] = 0.0f;
	//el[0][3] = 0.0f; el[1][3] = 0.0f; el[2][3] = 0.0f; el[3][3] = digo;
}
Mat4::Mat4(const Vec4& col0, const Vec4& col1, const Vec4& col2, const Vec4& col3) {
	el[0][0] = col0.x; el[1][0] = col1.x; el[2][0] = col2.x; el[3][0] = col3.x;
	el[0][1] = col0.y; el[1][1] = col1.y; el[2][1] = col2.y; el[3][1] = col3.y;
	el[0][2] = col0.z; el[1][2] = col1.z; el[2][2] = col2.z; el[3][2] = col3.z;
	el[0][3] = col0.w; el[1][3] = col1.w; el[2][3] = col2.w; el[3][3] = col3.w;
}

auto Mat4::operator[](const int index) -> std::array<float, 4>& {
	return this->el[index];
}

auto Mat4::operator[](const int index) const -> const std::array<float, 4>& {
	return this->el[index];
}

//auto Mat4::operator*(const Vec3& vector) const -> Vec3{
//	Vec3 result;
//	result.x = el[0][0] * vector.x + el[1][0] * vector.y + el[2][0] * vector.z + el[3][0] * 1;
//	result.y = el[0][1] * vector.x + el[1][1] * vector.y + el[2][1] * vector.z + el[3][1] * 1;
//	result.z = el[0][2] * vector.x + el[1][2] * vector.y + el[2][2] * vector.z + el[3][2] * 1;
//	return result;
//}

auto Mat4::operator*(const Vec4& vector) const -> Vec4 {
	Vec4 result;
	result.x = el[0][0] * vector.x + el[1][0] * vector.y + el[2][0] * vector.z + el[3][0] * vector.w;
	result.y = el[0][1] * vector.x + el[1][1] * vector.y + el[2][1] * vector.z + el[3][1] * vector.w;
	result.z = el[0][2] * vector.x + el[1][2] * vector.y + el[2][2] * vector.z + el[3][2] * vector.w;
	result.w = el[0][3] * vector.x + el[1][3] * vector.y + el[2][3] * vector.z + el[3][3] * vector.w;
	return result;
}

auto Mat4::operator*(const Mat4& other) const -> Mat4 {
	Mat4 result(0.0f);
	for(int row = 0; row < 4; ++row) {
		for(int col = 0; col < 4; ++col) {
			for(unsigned int k = 0; k < 4; ++k) {
				//result.el[col][row] += el[k][row] * other.el[col][k];
				result.el[col][row] += el[col][k] * other.el[k][row];
			}
		}
	}
	return result;
}

auto Mat4::ortho(float left, float right, float bottom, float top, float near, float far) -> Mat4 {
	Mat4 result(1.0f);
	result.el[0][0] = 2.0f / (right - left);
	result.el[1][1] = 2.0f / (top - bottom);
	result.el[2][2] = -2.0f / (far - near);
	result.el[3][0] = -(right + left) / (right - left);
	result.el[3][1] = -(top + bottom) / (top - bottom);
	result.el[3][2] = -(far + near) / (far - near);
	return result;
}

auto Mat4::perspective(float fovy, float aspect, float near, float far) -> Mat4 {
	float const tan_half_fovy = tan(fovy / 2);

	Mat4 result(0.0f);
	result.el[0][0] = 1.0f / (aspect * tan_half_fovy);
	result.el[1][1] = 1.0f / tan_half_fovy;
	result.el[2][2] = -(far + near) / (far - near);
	result.el[2][3] = -1.0f;
	result.el[3][2] = -(2 * far * near) / (far - near);
	return result;
}

auto Mat4::translate(Mat4 const& mat, const Vec3& trans) -> Mat4 {
	return Mat4::translate(trans) * mat;
}
auto Mat4::translate(const Vec3& trans) -> Mat4 {
	Mat4 result(1.0f);
	result.el[3][0] = trans.x;
	result.el[3][1] = trans.y;
	result.el[3][2] = trans.z;
	return result;
}

auto Mat4::rotate(Mat4 const& mat, float angle, const Vec3& axis) -> Mat4 {
	return Mat4::rotate(angle, axis) * mat;
}
auto Mat4::rotate(float angle, const Vec3& axis) -> Mat4 {

	float c = cos(angle);
	float omc = 1 - c;
	float s = sin(angle);

	Mat4 result(1.0f);
	result.el[0][0] = axis.x * axis.x * omc + c;
	result.el[0][1] = axis.y * axis.x * omc + axis.z * s;
	result.el[0][2] = axis.z * axis.x * omc - axis.y * s;

	result.el[1][0] = axis.x * axis.y * omc - axis.z * s;
	result.el[1][1] = axis.y * axis.y * omc + c;
	result.el[1][2] = axis.z * axis.y * omc + axis.x * s;

	result.el[2][0] = axis.x * axis.z * omc + axis.y * s;
	result.el[2][1] = axis.y * axis.z * omc - axis.x * s;
	result.el[2][2] = axis.z * axis.z * omc + c;
	return result;
}

auto Mat4::scale(Mat4 const& mat, const Vec3& scale) -> Mat4 {
	return Mat4::scale(scale) * mat;
}
auto Mat4::scale(const Vec3& scale) -> Mat4 {
	Mat4 result(1.0f);
	result.el[0][0] = scale.x;
	result.el[1][1] = scale.y;
	result.el[2][2] = scale.z;
	return result;
}

/*
+a_00*a_11*a_22*a_33
-a_00*a_11*a_32*a_23
-a_00*a_21*a_12*a_33
+a_00*a_21*a_32*a_13
+a_00*a_31*a_12*a_23
-a_00*a_31*a_22*a_13

-a_10*a_01*a_22*a_33
+a_10*a_01*a_32*a_23
+a_10*a_21*a_02*a_33
-a_10*a_21*a_32*a_03
-a_10*a_31*a_02*a_23
+a_10*a_31*a_22*a_03

+a_20*a_01*a_12*a_33
-a_20*a_01*a_32*a_13
-a_20*a_11*a_02*a_33
+a_20*a_11*a_32*a_03
+a_20*a_31*a_02*a_13
-a_20*a_31*a_12*a_03

-a_30*a_01*a_12*a_23
+a_30*a_01*a_22*a_13
+a_30*a_11*a_02*a_23
-a_30*a_11*a_22*a_03
-a_30*a_21*a_02*a_13
+a_30*a_21*a_12*a_03
*/

auto Mat4::get_determinant() const -> float {
	const Mat4& m = *this;
	float t00 = m[0][0] * m[1][1] * m[2][2] * m[3][3];
	float t01 = m[0][0] * m[1][1] * m[3][2] * m[2][3];
	float t02 = m[0][0] * m[2][1] * m[1][2] * m[3][3];
	float t03 = m[0][0] * m[2][1] * m[3][2] * m[1][3];
	float t04 = m[0][0] * m[3][1] * m[1][2] * m[2][3];
	float t05 = m[0][0] * m[3][1] * m[2][2] * m[1][3];
	float t0 = + t00 - t01 - t02 + t03 + t04 - t05;

	float t10 = m[1][0] * m[0][1] * m[2][2] * m[3][3];
	float t11 = m[1][0] * m[0][1] * m[3][2] * m[2][3];
	float t12 = m[1][0] * m[2][1] * m[0][2] * m[3][3];
	float t13 = m[1][0] * m[2][1] * m[3][2] * m[0][3];
	float t14 = m[1][0] * m[3][1] * m[0][2] * m[2][3];
	float t15 = m[1][0] * m[3][1] * m[2][2] * m[0][3];
	float t1 = - t10 + t11 + t12 - t13 - t14 + t15;

	float t20 = m[2][0] * m[0][1] * m[1][2] * m[3][3];
	float t21 = m[2][0] * m[0][1] * m[3][2] * m[1][3];
	float t22 = m[2][0] * m[1][1] * m[0][2] * m[3][3];
	float t23 = m[2][0] * m[1][1] * m[3][2] * m[0][3];
	float t24 = m[2][0] * m[3][1] * m[0][2] * m[1][3];
	float t25 = m[2][0] * m[3][1] * m[1][2] * m[0][3];
	float t2 = +t20 - t21 - t22 + t23 + t24 - t25;

	float t30 = m[3][0] * m[0][1] * m[1][2] * m[2][3];
	float t31 = m[3][0] * m[0][1] * m[2][2] * m[1][3];
	float t32 = m[3][0] * m[1][1] * m[0][2] * m[2][3];
	float t33 = m[3][0] * m[1][1] * m[2][2] * m[0][3];
	float t34 = m[3][0] * m[2][1] * m[0][2] * m[1][3];
	float t35 = m[3][0] * m[2][1] * m[1][2] * m[0][3];
	float t3 = -t30 + t31 + t32 - t33 - t34 + t35;

	float t = t0 + t1 + t2 + t3;

	return t;
}


struct Matrix4x4 {
	union {
		std::array<std::array<float, 4>, 4> el;
		std::array<float, 4> colVec;
	};
	Matrix4x4(const float digo) {
		for (int col = 0; col < 4; col++) {
			for (int row = 0; row < 4; row++) {
				el[col][row] = (col == row) ? float(digo) : float(0.0f);
			}
		}
	}

	static auto add(const Matrix4x4& mat0, const Matrix4x4& mat1) -> Matrix4x4 {
		Matrix4x4 result(0.0f);
		for (int row = 0; row < 4; ++row) {
			for (int col = 0; col < 4; ++col) {
				result.el[col][row] = mat0.el[col][row] + mat1.el[col][row];
			}
		}
		return result;
	}
	static auto scalar_mult(const float scalar, Matrix4x4 mat) -> Matrix4x4 {
		Matrix4x4 result(0.0f);
		for (int row = 0; row < 4; ++row) {
			for (int col = 0; col < 4; ++col) {
				result.el[col][row] = scalar * mat.el[col][row];
			}
		}
		return result;
	}
	static auto transpose(Matrix4x4 mat) -> Matrix4x4 {
		Matrix4x4 result(0.0f);
		for (int row = 0; row < 4; ++row) {
			for (int col = 0; col < 4; ++col) {
				result.el[col][row] = mat.el[row][col];
			}
		}
		return result;
	}
	static auto dot_product(const Matrix4x4& mat0, const Matrix4x4& mat1) -> Matrix4x4 {
		Matrix4x4 result(0.0f);
		for (int row = 0; row < 4; ++row) {
			for (int col = 0; col < 4; ++col) {
				for (unsigned int k = 0; k < 4; ++k) {
					result.el[col][row] += mat0.el[col][k] * mat1.el[k][row];
				}
			}
		}
		return result;
	}
	static auto dot_product(const Matrix4x4& mat, const Vec4& vector) -> Vec4 {

		Vec4 result;
		result.x = mat.el[0][0] * vector.x + mat.el[1][0] * vector.y + mat.el[2][0] * vector.z + mat.el[3][0] * vector.w;
		result.y = mat.el[0][1] * vector.x + mat.el[1][1] * vector.y + mat.el[2][1] * vector.z + mat.el[3][1] * vector.w;
		result.z = mat.el[0][2] * vector.x + mat.el[1][2] * vector.y + mat.el[2][2] * vector.z + mat.el[3][2] * vector.w;
		result.w = mat.el[0][3] * vector.x + mat.el[1][3] * vector.y + mat.el[2][3] * vector.z + mat.el[3][3] * vector.w;
		return result;
	}
	auto operator*(const Matrix4x4& other) const -> Matrix4x4 {
		return Matrix4x4::dot_product(*this, other);
	}
};