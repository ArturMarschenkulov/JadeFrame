#include "Mat4.h"

Mat4::Mat4() {
	for (int col = 0; col < 4; col++) {
		for (int row = 0; row < 4; row++) {
			el[col][row] = (col == row) ? float(1.0f) : float(0.0f);
		}
	}
	//el[0][0] = 1.0f; el[1][0] = 0.0f; el[2][0] = 0.0f; el[3][0] = 0.0f;
	//el[0][1] = 0.0f; el[1][1] = 1.0f; el[2][1] = 0.0f; el[3][1] = 0.0f;
	//el[0][2] = 0.0f; el[1][2] = 0.0f; el[2][2] = 1.0f; el[3][2] = 0.0f;
	//el[0][3] = 0.0f; el[1][3] = 0.0f; el[2][3] = 0.0f; el[3][3] = 1.0f;
}
Mat4::Mat4(float digo) {
	for (int col = 0; col < 4; col++) {
		for (int row = 0; row < 4; row++) {
			el[col][row] = (col == row) ? float(digo) : float(0.0f);
		}
	}
	//el[0][0] = digo; el[1][0] = 0.0f; el[2][0] = 0.0f; el[3][0] = 0.0f;
	//el[0][1] = 0.0f; el[1][1] = digo; el[2][1] = 0.0f; el[3][1] = 0.0f;
	//el[0][2] = 0.0f; el[1][2] = 0.0f; el[2][2] = digo; el[3][2] = 0.0f;
	//el[0][3] = 0.0f; el[1][3] = 0.0f; el[2][3] = 0.0f; el[3][3] = digo;
}
Mat4::Mat4(Vec4 col0, Vec4 col1, Vec4 col2, Vec4 col3) {
	el[0][0] = col0.x; el[1][0] = col1.x; el[2][0] = col2.x; el[3][0] = col3.x;
	el[0][1] = col0.y; el[1][1] = col1.y; el[2][1] = col2.y; el[3][1] = col3.y;
	el[0][2] = col0.z; el[1][2] = col1.z; el[2][2] = col2.z; el[3][2] = col3.z;
	el[0][3] = col0.w; el[1][3] = col1.w; el[2][3] = col2.w; el[3][3] = col3.w;
}

std::array<float, 4>& Mat4::operator[](const int index) {
	return this->el[index];
}

const std::array<float, 4>& Mat4::operator[](const int index) const {

	return this->el[index];
}

Vec3 Mat4::operator*(const Vec3& vector) const {
	Vec3 result;
	result.x = el[0][0] * vector.x + el[1][0] * vector.y + el[2][0] * vector.z;
	result.y = el[0][1] * vector.x + el[1][1] * vector.y + el[2][1] * vector.z;
	result.z = el[0][2] * vector.x + el[1][2] * vector.y + el[2][2] * vector.z;
	return result;
}

Vec4 Mat4::operator*(const Vec4& vector) const {
	Vec4 result;
	result.x = el[0][0] * vector.x + el[1][0] * vector.y + el[2][0] * vector.z + el[3][0] * vector.w;
	result.y = el[0][1] * vector.x + el[1][1] * vector.y + el[2][1] * vector.z + el[3][1] * vector.w;
	result.z = el[0][2] * vector.x + el[1][2] * vector.y + el[2][2] * vector.z + el[3][2] * vector.w;
	result.w = el[0][3] * vector.x + el[1][3] * vector.y + el[2][3] * vector.z + el[3][3] * vector.w;
	return result;
}

Mat4 Mat4::operator*(const Mat4& other) const {
	Mat4 result(0.0f);
	for ( int row = 0; row < 4; ++row ) {
		for ( int col = 0; col < 4; ++col ) {
			float sum = 0.0f;
			for ( unsigned int k = 0; k < 4; ++k ) {
				//result.el[col][row] += el[k][row] * other.el[col][k];
				result.el[col][row] += el[col][k] * other.el[k][row];
			}
		}
	}
	return result;
}

Mat4 Mat4::ortho(float left, float right, float bottom, float top, float near, float far) {
	Mat4 result(1.0f);
	result.el[0][0] = 2.0f / ( right - left );
	result.el[1][1] = 2.0f / ( top - bottom );
	result.el[2][2] = -2.0f / ( far - near );
	result.el[3][0] = -( right + left ) / ( right - left );
	result.el[3][1] = -( top + bottom ) / ( top - bottom );
	result.el[3][2] = -( far + near ) / ( far - near );
	return result;
}

Mat4 Mat4::perspective(float fovy, float aspect, float near, float far) {
	float const tanHalfFovy = tan(fovy / 2);

	Mat4 result(0.0f);
	result.el[0][0] = 1.0f / ( aspect * tanHalfFovy );
	result.el[1][1] = 1.0f / tanHalfFovy;
	result.el[2][2] = -( far + near ) / ( far - near );
	result.el[2][3] = -1.0f;
	result.el[3][2] = -( 2 * far * near ) / ( far - near );
	return result;
}

Mat4 Mat4::translate(Mat4 const& mat, const Vec3& trans) {
	return Mat4::translate(trans) * mat;
}
Mat4 Mat4::translate(const Vec3& trans) {
	Mat4 result(1.0f);
	result.el[3][0] = trans.x;
	result.el[3][1] = trans.y;
	result.el[3][2] = trans.z;
	return result;
}

Mat4 Mat4::rotate(Mat4 const& mat, float angle, const Vec3& axis) {
	return Mat4::rotate(angle, axis) * mat;
}
Mat4 Mat4::rotate(float angle, const Vec3& axis) {

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

Mat4 Mat4::scale(Mat4 const& mat, const Vec3& scale) {
	return Mat4::scale(scale) * mat;
}
Mat4 Mat4::scale(const Vec3& scale) {
	Mat4 result(1.0f);
	result.el[0][0] = scale.x;
	result.el[1][1] = scale.y;
	result.el[2][2] = scale.z;
	return result;
}
