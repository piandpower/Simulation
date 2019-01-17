#include "Matrix3D.h"

Matrix3D::Matrix3D() : 
	S11(0.0), 
	S12(0.0), 
	S13(0.0), 
	S21(0.0), 
	S22(0.0), 
	S23(0.0), 
	S31(0.0), 
	S32(0.0), 
	S33(0.0)
{
}

Matrix3D::Matrix3D(const double s11, const double s12, const double s13, const double s21, const double s22, const double s23, const double s31, const double s32, const double s33) : 
	S11(s11),
	S12(s12),
	S13(s13),
	S21(s21),
	S22(s22),
	S23(s23),
	S31(s31),
	S32(s32),
	S33(s33)
{
}

Matrix3D::Matrix3D(const float s11, const float s12, const float s13, const float s21, const float s22, const float s23, const float s31, const float s32, const float s33) :
	S11(s11),
	S12(s12),
	S13(s13),
	S21(s21),
	S22(s22),
	S23(s23),
	S31(s31),
	S32(s32),
	S33(s33)
{
}

Matrix3D::Matrix3D(const int s11, const int s12, const int s13, const int s21, const int s22, const int s23, const int s31, const int s32, const int s33) : 
	S11(s11),
	S12(s12),
	S13(s13),
	S21(s21),
	S22(s22),
	S23(s23),
	S31(s31),
	S32(s32),
	S33(s33)
{
}

Matrix3D::Matrix3D(const Vector3D & v1, const Vector3D & v2, const Vector3D & v3) : 
	S11(v1.X),
	S12(v1.Y),
	S13(v1.Z),
	S21(v2.X),
	S22(v2.Y),
	S23(v2.Z),
	S31(v3.X),
	S32(v3.Y),
	S33(v3.Z)
{
}

Matrix3D::Matrix3D(const FVector & v1, const FVector & v2, const FVector & v3) :
	S11(v1.X),
	S12(v1.Y),
	S13(v1.Z),
	S21(v2.X),
	S22(v2.Y),
	S23(v2.Z),
	S31(v3.X),
	S32(v3.Y),
	S33(v3.Z)
{
}

Matrix3D::~Matrix3D()
{
}

void Matrix3D::operator+=(const Matrix3D & m)
{
	S11 += m.S11;
	S12 += m.S12;
	S13 += m.S13;
	S21 += m.S21;
	S22 += m.S22;
	S23 += m.S23;
	S31 += m.S31;
	S32 += m.S32;
	S33 += m.S33;
}

void Matrix3D::operator-=(const Matrix3D & m)
{
	S11 -= m.S11;
	S12 -= m.S12;
	S13 -= m.S13;
	S21 -= m.S21;
	S22 -= m.S22;
	S23 -= m.S23;
	S31 -= m.S31;
	S32 -= m.S32;
	S33 -= m.S33;
}

void Matrix3D::operator*=(const double s)
{
	S11 *= s;
	S12 *= s;
	S13 *= s;
	S21 *= s;
	S22 *= s;
	S23 *= s;
	S31 *= s;
	S32 *= s;
	S33 *= s;
}

void Matrix3D::operator/=(const double s)
{
	S11 /= s;
	S12 /= s;
	S13 /= s;
	S21 /= s;
	S22 /= s;
	S23 /= s;
	S31 /= s;
	S32 /= s;
	S33 /= s;
}

void Matrix3D::operator*=(const Matrix3D & m)
{
	Matrix3D temp;
	temp.S11 = S11 * m.S11 + S12 * m.S21 + S13 * m.S31;
	temp.S12 = S11 * m.S12 + S12 * m.S22 + S13 * m.S32;
	temp.S13 = S11 * m.S13 + S12 * m.S23 + S13 * m.S33;
	temp.S21 = S21 * m.S11 + S22 * m.S21 + S23 * m.S31;
	temp.S22 = S21 * m.S12 + S22 * m.S22 + S23 * m.S32;
	temp.S23 = S21 * m.S13 + S22 * m.S23 + S23 * m.S33;
	temp.S31 = S31 * m.S11 + S32 * m.S21 + S33 * m.S31;
	temp.S32 = S31 * m.S12 + S32 * m.S22 + S33 * m.S32;
	temp.S33 = S31 * m.S13 + S32 * m.S23 + S33 * m.S33;

	*this = temp;
}

void Matrix3D::operator=(const Matrix3D & m)
{
	S11 = m.S11;
	S12 = m.S12;
	S13 = m.S13;
	S21 = m.S21;
	S22 = m.S22;
	S23 = m.S23;
	S31 = m.S31;
	S32 = m.S32;
	S33 = m.S33;
}

Matrix3D Matrix3D::ElementwiseMultiplication(const Matrix3D & m1, const Matrix3D & m2)
{
	Matrix3D temp;
	temp.S11 = m1.S11 * m2.S11;
	temp.S12 = m1.S12 * m2.S12;
	temp.S13 = m1.S13 * m2.S13;
	temp.S21 = m1.S21 * m2.S21;
	temp.S22 = m1.S22 * m2.S22;
	temp.S23 = m1.S23 * m2.S23;
	temp.S31 = m1.S31 * m2.S31;
	temp.S32 = m1.S32 * m2.S32;
	temp.S33 = m1.S33 * m2.S33;

	return temp;
}

Matrix3D Matrix3D::OuterProduct(const Vector3D & v1, const Vector3D & v2)
{
	return Matrix3D(v1.X * v2.X, v1.X * v2.Y, v1.X * v2.Z, 
					v1.Y * v2.X, v1.Y * v2.Y, v1.Y * v2.Z,
					v1.Z * v2.X, v1.Z * v2.Y, v1.Z * v2.Z);
}

void Matrix3D::Transpose()
{
	std::swap(S21, S12);
	std::swap(S31, S13);
	std::swap(S32, S23);
}

void Matrix3D::Inverse()
{
	*this = GetInverse();
}

Matrix3D Matrix3D::GetTransposed() const
{
	return Matrix3D(S11, S21, S31,
					S12, S22, S32,
					S13, S23, S33);
}

Matrix3D Matrix3D::GetInverse() const
{
	double d = 1.0 / GetDeterminant();
	return Matrix3D((S22 * S33 - S32 * S23) * d, (S32 * S13 - S12 * S33) * d, (S12 * S23 - S22 * S13) * d,
					(S23 * S31 - S33 * S21) * d, (S33 * S11 - S13 * S31) * d, (S13 * S21 - S23 * S11) * d,
					(S21 * S32 - S31 * S22) * d, (S31 * S12 - S11 * S32) * d, (S11 * S22 - S21 * S12) * d);
}

double Matrix3D::GetDeterminant() const
{
	return	S11 * S22 * S33 +
			S12 * S23 * S31 + 
			S13 * S21 * S32 -
			S13 * S22 * S31 -
			S12 * S21 * S33 -
			S11 * S23 * S32;
}

bool Matrix3D::IsInvertible() const
{
	return GetDeterminant() != 0;
}

const Matrix3D Matrix3D::Identity = {	1.0, 0.0, 0.0,
										0.0, 1.0, 0.0,
										0.0, 0.0, 1.0 };

const Matrix3D Matrix3D::Zero = {	0.0, 0.0, 0.0,
									0.0, 0.0, 0.0, 
									0.0, 0.0, 0.0 };


bool operator==(const Matrix3D & m1, const Matrix3D & m2)
{
	return (
		m1.S11 == m2.S11 &&
		m1.S12 == m2.S12 &&
		m1.S13 == m2.S13 &&
		m1.S21 == m2.S21 &&
		m1.S22 == m2.S22 &&
		m1.S23 == m2.S23 &&
		m1.S31 == m2.S31 &&
		m1.S32 == m2.S32 &&
		m1.S33 == m2.S33);
}

bool operator!=(const Matrix3D & m1, const Matrix3D & m2)
{
	return !operator==(m1, m2);
}

Matrix3D operator+(const Matrix3D & m1, const Matrix3D & m2)
{
	return Matrix3D(
		m1.S11 + m2.S11,
		m1.S12 + m2.S12,
		m1.S13 + m2.S13,
		m1.S21 + m2.S21,
		m1.S22 + m2.S22,
		m1.S23 + m2.S23,
		m1.S31 + m2.S31,
		m1.S32 + m2.S32,
		m1.S33 + m2.S33);
}

Matrix3D operator-(const Matrix3D & m1, const Matrix3D & m2)
{
	return Matrix3D(
		m1.S11 - m2.S11,
		m1.S12 - m2.S12,
		m1.S13 - m2.S13,
		m1.S21 - m2.S21,
		m1.S22 - m2.S22,
		m1.S23 - m2.S23,
		m1.S31 - m2.S31,
		m1.S32 - m2.S32,
		m1.S33 - m2.S33);
}

Matrix3D operator-(const Matrix3D & m)
{
	return Matrix3D(-m.S11, -m.S12, -m.S13, -m.S21, -m.S22, -m.S23, -m.S31, -m.S32, -m.S33);
}

Matrix3D operator*(const Matrix3D & m, const double s)
{
	return Matrix3D(
		m.S11 * s,
		m.S12 * s,
		m.S13 * s,
		m.S21 * s,
		m.S22 * s,
		m.S23 * s,
		m.S31 * s,
		m.S32 * s,
		m.S33 * s);
}

Matrix3D operator*(const double s, const Matrix3D & m)
{
	return Matrix3D(
		m.S11 * s,
		m.S12 * s,
		m.S13 * s,
		m.S21 * s,
		m.S22 * s,
		m.S23 * s,
		m.S31 * s,
		m.S32 * s,
		m.S33 * s);
}

Matrix3D operator/(const Matrix3D & m, const double s)
{
	return Matrix3D(
		m.S11 / s,
		m.S12 / s,
		m.S13 / s,
		m.S21 / s,
		m.S22 / s,
		m.S23 / s,
		m.S31 / s,
		m.S32 / s,
		m.S33 / s);
}

Matrix3D operator*(const Matrix3D & m1, const Matrix3D & m2)
{
	return Matrix3D(
		m1.S11 * m2.S11 + m1.S12 * m2.S21 + m1.S13 * m2.S31,
		m1.S11 * m2.S12 + m1.S12 * m2.S22 + m1.S13 * m2.S32,
		m1.S11 * m2.S13 + m1.S12 * m2.S23 + m1.S13 * m2.S33,
		m1.S21 * m2.S11 + m1.S22 * m2.S21 + m1.S23 * m2.S31,
		m1.S21 * m2.S12 + m1.S22 * m2.S22 + m1.S23 * m2.S32,
		m1.S21 * m2.S13 + m1.S22 * m2.S23 + m1.S23 * m2.S33,
		m1.S31 * m2.S11 + m1.S32 * m2.S21 + m1.S33 * m2.S31,
		m1.S31 * m2.S12 + m1.S32 * m2.S22 + m1.S33 * m2.S32,
		m1.S31 * m2.S13 + m1.S32 * m2.S23 + m1.S33 * m2.S33);
}

Vector3D operator*(const Matrix3D & m, const Vector3D & v)
{
	return Vector3D(
		m.S11 * v.X + m.S12 * v.Y + m.S13 * v.Z,
		m.S21 * v.X + m.S22 * v.Y + m.S23 * v.Z,
		m.S31 * v.X + m.S32 * v.Y + m.S33 * v.Z	);
}
