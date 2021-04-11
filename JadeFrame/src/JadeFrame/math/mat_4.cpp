#include "mat_4.h"

Matrix4x4::Matrix4x4()
	:el() {

	el[0][0] = 1.0f; el[1][0] = 0.0f; el[2][0] = 0.0f; el[3][0] = 0.0f;
	el[0][1] = 0.0f; el[1][1] = 1.0f; el[2][1] = 0.0f; el[3][1] = 0.0f;
	el[0][2] = 0.0f; el[1][2] = 0.0f; el[2][2] = 1.0f; el[3][2] = 0.0f;
	el[0][3] = 0.0f; el[1][3] = 0.0f; el[2][3] = 0.0f; el[3][3] = 1.0f;
}
Matrix4x4::Matrix4x4(const float digo) {
	el[0][0] = digo; el[1][0] = 0.0f; el[2][0] = 0.0f; el[3][0] = 0.0f;
	el[0][1] = 0.0f; el[1][1] = digo; el[2][1] = 0.0f; el[3][1] = 0.0f;
	el[0][2] = 0.0f; el[1][2] = 0.0f; el[2][2] = digo; el[3][2] = 0.0f;
	el[0][3] = 0.0f; el[1][3] = 0.0f; el[2][3] = 0.0f; el[3][3] = digo;
}
Matrix4x4::Matrix4x4(const Vec4& col0, const Vec4& col1, const Vec4& col2, const Vec4& col3) {
	el[0][0] = col0.x; el[1][0] = col1.x; el[2][0] = col2.x; el[3][0] = col3.x;
	el[0][1] = col0.y; el[1][1] = col1.y; el[2][1] = col2.y; el[3][1] = col3.y;
	el[0][2] = col0.z; el[1][2] = col1.z; el[2][2] = col2.z; el[3][2] = col3.z;
	el[0][3] = col0.w; el[1][3] = col1.w; el[2][3] = col2.w; el[3][3] = col3.w;
}

Matrix4x4::Matrix4x4(const Matrix4x4& mat) {
	*this = mat;
}

auto Matrix4x4::operator=(const Matrix4x4& mat) -> Matrix4x4& {
	for (int row = 0; row < 4; row++) {
		for (int col = 0; col < 4; col++) {
			el[row][col] = mat[row][col];
		}
	}
	return (*this);
}

auto Matrix4x4::operator[](const int index)->std::array<float, 4>& {
	return this->el[index];
}

auto Matrix4x4::operator[](const int index) const -> const std::array<float, 4>& {
	return this->el[index];
}

//auto Matrix4x4::operator*(const Vec3& vector) const -> Vec3{
//	Vec3 result;
//	result.x = el[0][0] * vector.x + el[1][0] * vector.y + el[2][0] * vector.z + el[3][0] * 1;
//	result.y = el[0][1] * vector.x + el[1][1] * vector.y + el[2][1] * vector.z + el[3][1] * 1;
//	result.z = el[0][2] * vector.x + el[1][2] * vector.y + el[2][2] * vector.z + el[3][2] * 1;
//	return result;
//}

auto Matrix4x4::operator*(const Vec4& vector) const -> Vec4 {
	Vec4 result;
	result.x = el[0][0] * vector.x + el[1][0] * vector.y + el[2][0] * vector.z + el[3][0] * vector.w;
	result.y = el[0][1] * vector.x + el[1][1] * vector.y + el[2][1] * vector.z + el[3][1] * vector.w;
	result.z = el[0][2] * vector.x + el[1][2] * vector.y + el[2][2] * vector.z + el[3][2] * vector.w;
	result.w = el[0][3] * vector.x + el[1][3] * vector.y + el[2][3] * vector.z + el[3][3] * vector.w;
	return result;
}

auto Matrix4x4::operator*(const Matrix4x4& other) const -> Matrix4x4 {
	Matrix4x4 result(0.0f);
	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			for (unsigned int k = 0; k < 4; ++k) {
				//result.el[col][row] += el[k][row] * other.el[col][k];
				result.el[col][row] += el[col][k] * other.el[k][row];
			}
		}
	}
	return result;
}

auto Matrix4x4::orthogonal_projection_matrix(float left, float right, float bottom, float top, float near, float far) -> Matrix4x4 {
	auto ortho_LH_ZO = [](float left, float right, float bottom, float top, float zNear, float zFar) -> Matrix4x4 {
		Matrix4x4 Result(1);
		Result[0][0] = static_cast<float>(2) / (right - left);
		Result[1][1] = static_cast<float>(2) / (top - bottom);
		Result[2][2] = static_cast<float>(1) / (zFar - zNear);
		Result[3][0] = -(right + left) / (right - left);
		Result[3][1] = -(top + bottom) / (top - bottom);
		Result[3][2] = -zNear / (zFar - zNear);
		return Result;
	};
	auto ortho_LH_NO = [](float left, float right, float bottom, float top, float zNear, float zFar) -> Matrix4x4 {
		Matrix4x4 Result(1);
		Result[0][0] = 2.0f / (right - left);
		Result[1][1] = 2.0f / (top - bottom);
		Result[2][2] = 2.0f / (zFar - zNear);
		Result[3][0] = -(right + left) / (right - left);
		Result[3][1] = -(top + bottom) / (top - bottom);
		Result[3][2] = -(zFar + zNear) / (zFar - zNear);
		return Result;
	};
	auto ortho_RH_ZO = [](float left, float right, float bottom, float top, float zNear, float zFar) -> Matrix4x4 {
		Matrix4x4 Result(1);
		Result[0][0] = 2.0 / (right - left);
		Result[1][1] = 2.0 / (top - bottom);
		Result[2][2] = -1.0 / (zFar - zNear);
		Result[3][0] = -(right + left) / (right - left);
		Result[3][1] = -(top + bottom) / (top - bottom);
		Result[3][2] = -zNear / (zFar - zNear);
		return Result;
	};
	auto ortho_RH_NO = [](float left, float right, float bottom, float top, float zNear, float zFar) -> Matrix4x4 {
		Matrix4x4 Result(1);
		Result[0][0] = 2.0f / (right - left);
		Result[1][1] = 2.0f / (top - bottom);
		Result[2][2] = -2.0f / (zFar - zNear);
		Result[3][0] = -(right + left) / (right - left);
		Result[3][1] = -(top + bottom) / (top - bottom);
		Result[3][2] = -(zFar + zNear) / (zFar - zNear);
		return Result;
	};
	//Matrix4x4 (*ortho_fn[])(float, float, float, float, float, float) = { ortho_LH_ZO, ortho_LH_NO,ortho_RH_ZO, ortho_RH_NO };
	//return ortho_fn[1](left, right, bottom, top, near, far);
	return ortho_RH_NO(left, right, bottom, top, near, far);

	//Matrix4x4 result(1.0f);
	//result.el[0][0] = 2.0f / (right - left);
	//result.el[1][1] = 2.0f / (top - bottom);
	//result.el[2][2] = -2.0f / (far - near);
	//result.el[3][0] = -(right + left) / (right - left);
	//result.el[3][1] = -(top + bottom) / (top - bottom);
	//result.el[3][2] = -(far + near) / (far - near);
	//return result;
}

auto Matrix4x4::perspective_projection_matrix(float fovy, float aspect, float near, float far) -> Matrix4x4 {
	const float tan_half_fovy = tan(fovy / 2.0f);
	Matrix4x4 result(0.0f);
	result.el[0][0] = 1.0f / (aspect * tan_half_fovy);
	result.el[1][1] = 1.0f / tan_half_fovy;
	result.el[2][2] = -(far + near) / (far - near);
	result.el[2][3] = -1.0f;
	result.el[3][2] = -(2 * far * near) / (far - near);
	return result;
}
auto Matrix4x4::translation_matrix(const Vec3& trans) -> Matrix4x4 {
	Matrix4x4 result(1.0f);
	result.el[3][0] = trans.x;
	result.el[3][1] = trans.y;
	result.el[3][2] = trans.z;
	return result;
}
auto Matrix4x4::rotation_matrix(float angle, const Vec3& axis) -> Matrix4x4 {
	const float c = cos(angle);
	const float omc = 1 - c;
	const float s = sin(angle);

	Matrix4x4 result(1.0f);
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
auto Matrix4x4::scale_matrix(const Vec3& scale) -> Matrix4x4 {
	Matrix4x4 result(1.0f);
	result.el[0][0] = scale.x;
	result.el[1][1] = scale.y;
	result.el[2][2] = scale.z;
	return result;
}

auto Matrix4x4::look_at_matrix(const Vec3& camera, Vec3 object, Vec3 up) -> Matrix4x4 {
	auto look_at_LH = [](const Vec3& eye, const Vec3& center, const Vec3& up) {
		Vec3 const f((center - eye).get_normal());
		Vec3 const s(up.cross(f).get_normal());
		Vec3 const u(f.cross(s));

		Matrix4x4 Result(1);
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

		Matrix4x4 Result(1);
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

	Matrix4x4 matrix;
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
	Matrix4x4 result;
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

	return result * Matrix4x4::translate(Vec3(-camera.x, -camera.y, -camera.z));
#endif

}

auto Matrix4x4::get_determinant() const -> float {
	const Matrix4x4& m = *this;
	float t00 = m[0][0] * m[1][1] * m[2][2] * m[3][3];
	float t01 = m[0][0] * m[1][1] * m[3][2] * m[2][3];
	float t02 = m[0][0] * m[2][1] * m[1][2] * m[3][3];
	float t03 = m[0][0] * m[2][1] * m[3][2] * m[1][3];
	float t04 = m[0][0] * m[3][1] * m[1][2] * m[2][3];
	float t05 = m[0][0] * m[3][1] * m[2][2] * m[1][3];

	float t10 = m[1][0] * m[0][1] * m[2][2] * m[3][3];
	float t11 = m[1][0] * m[0][1] * m[3][2] * m[2][3];
	float t12 = m[1][0] * m[2][1] * m[0][2] * m[3][3];
	float t13 = m[1][0] * m[2][1] * m[3][2] * m[0][3];
	float t14 = m[1][0] * m[3][1] * m[0][2] * m[2][3];
	float t15 = m[1][0] * m[3][1] * m[2][2] * m[0][3];

	float t20 = m[2][0] * m[0][1] * m[1][2] * m[3][3];
	float t21 = m[2][0] * m[0][1] * m[3][2] * m[1][3];
	float t22 = m[2][0] * m[1][1] * m[0][2] * m[3][3];
	float t23 = m[2][0] * m[1][1] * m[3][2] * m[0][3];
	float t24 = m[2][0] * m[3][1] * m[0][2] * m[1][3];
	float t25 = m[2][0] * m[3][1] * m[1][2] * m[0][3];

	float t30 = m[3][0] * m[0][1] * m[1][2] * m[2][3];
	float t31 = m[3][0] * m[0][1] * m[2][2] * m[1][3];
	float t32 = m[3][0] * m[1][1] * m[0][2] * m[2][3];
	float t33 = m[3][0] * m[1][1] * m[2][2] * m[0][3];
	float t34 = m[3][0] * m[2][1] * m[0][2] * m[1][3];
	float t35 = m[3][0] * m[2][1] * m[1][2] * m[0][3];

	float t0 = +t00 - t01 - t02 + t03 + t04 - t05;
	float t1 = -t10 + t11 + t12 - t13 - t14 + t15;
	float t2 = +t20 - t21 - t22 + t23 + t24 - t25;
	float t3 = -t30 + t31 + t32 - t33 - t34 + t35;

	float t = t0 + t1 + t2 + t3;

	return t;
}

auto Matrix4x4::get_echelon() const -> Matrix4x4 {
	Matrix4x4 m = *this;
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

auto Matrix4x4::is_invertible() const -> bool {
	return (this->get_determinant() == 0) ? false : true;
}

auto Matrix4x4::get_rank() const -> int {
	int result = 0;
	auto e = this->get_echelon();
	for (int col = 0; col < 4; col++) {
		for (int row = 0; row < 4; row++) {
			result += (e[col][row] > 0) ? 1 : 0;
		}
	}
	return result;
}

auto Matrix4x4::get_transpose() const -> Matrix4x4 {
	Matrix4x4 result;
	for (int col = 0; col < 4; col++) {
		for (int row = 0; row < 4; row++) {
			result[col][row] = el[row][col];
		}
	}
	return result;
}

auto Matrix4x4::make_echelon() -> Matrix4x4& {
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

//
//struct Matrix4x4 {
//	union {
//		std::array<std::array<float, 4>, 4> el;
//		std::array<float, 4> colVec;
//	};
//	Matrix4x4(const float digo) {
//		for (int col = 0; col < 4; col++) {
//			for (int row = 0; row < 4; row++) {
//				el[col][row] = (col == row) ? float(digo) : float(0.0f);
//			}
//		}
//	}
//
//	static auto add(const Matrix4x4& mat0, const Matrix4x4& mat1) -> Matrix4x4 {
//		Matrix4x4 result(0.0f);
//		for (int row = 0; row < 4; ++row) {
//			for (int col = 0; col < 4; ++col) {
//				result.el[col][row] = mat0.el[col][row] + mat1.el[col][row];
//			}
//		}
//		return result;
//	}
//	static auto scalar_mult(const float scalar, Matrix4x4 mat) -> Matrix4x4 {
//		Matrix4x4 result(0.0f);
//		for (int row = 0; row < 4; ++row) {
//			for (int col = 0; col < 4; ++col) {
//				result.el[col][row] = scalar * mat.el[col][row];
//			}
//		}
//		return result;
//	}
//	static auto transpose(Matrix4x4 mat) -> Matrix4x4 {
//		Matrix4x4 result(0.0f);
//		for (int row = 0; row < 4; ++row) {
//			for (int col = 0; col < 4; ++col) {
//				result.el[col][row] = mat.el[row][col];
//			}
//		}
//		return result;
//	}
//	static auto dot_product(const Matrix4x4& mat0, const Matrix4x4& mat1) -> Matrix4x4 {
//		Matrix4x4 result(0.0f);
//		for (int row = 0; row < 4; ++row) {
//			for (int col = 0; col < 4; ++col) {
//				for (unsigned int k = 0; k < 4; ++k) {
//					result.el[col][row] += mat0.el[col][k] * mat1.el[k][row];
//				}
//			}
//		}
//		return result;
//	}
//	static auto dot_product(const Matrix4x4& mat, const Vec4& vector) -> Vec4 {
//
//		Vec4 result;
//		result.x = mat.el[0][0] * vector.x + mat.el[1][0] * vector.y + mat.el[2][0] * vector.z + mat.el[3][0] * vector.w;
//		result.y = mat.el[0][1] * vector.x + mat.el[1][1] * vector.y + mat.el[2][1] * vector.z + mat.el[3][1] * vector.w;
//		result.z = mat.el[0][2] * vector.x + mat.el[1][2] * vector.y + mat.el[2][2] * vector.z + mat.el[3][2] * vector.w;
//		result.w = mat.el[0][3] * vector.x + mat.el[1][3] * vector.y + mat.el[2][3] * vector.z + mat.el[3][3] * vector.w;
//		return result;
//	}
//	auto operator*(const Matrix4x4& other) const -> Matrix4x4 {
//		return Matrix4x4::dot_product(*this, other);
//	}
//};