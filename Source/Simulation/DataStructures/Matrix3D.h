#pragma once

#include <algorithm>

#include "Vector3D.h"


#include "CoreMinimal.h"

// 3x3 matrix
class Matrix3D {

public:
	double S11;
	double S12;
	double S13;

	double S21;
	double S22;
	double S23;

	double S31;
	double S32;
	double S33;


	// constructor/destructor
	Matrix3D();
	Matrix3D(const double s11, const double s12, const double s13, const double s21, const double s22, const double s23, const double s31, const double s32, const double s33);
	Matrix3D(const float s11, const float s12, const float s13, const float s21, const float s22, const float s23, const float s31, const float s32, const float s33);
	Matrix3D(const int s11, const int s12, const int s13, const int s21, const int s22, const int s23, const int s31, const int s32, const int s33);
	Matrix3D(const Vector3D& v1, const Vector3D& v2, const Vector3D& v3);
	Matrix3D(const FVector& v1, const FVector& v2, const FVector& v3);

	~Matrix3D();

	void operator +=(const Matrix3D& m);
	void operator -=(const Matrix3D& m);
	void operator *=(const double s);
	void operator /=(const double s);
	void operator *=(const Matrix3D& m);
	
	void operator =(const Matrix3D& m);

	static Matrix3D ElementwiseMultiplication(const Matrix3D & m1, const Matrix3D& m2);
	static Matrix3D OuterProduct(const Vector3D& v1, const Vector3D& v2);

	// transposes the matrix in place
	void Transpose();

	// Inverts the matrix
	void Inverse();

	// returns the transposed matrix
	Matrix3D GetTransposed() const;

	// returns the inverse matrix
	Matrix3D GetInverse() const;

	// Calculates the determinant of the matrix
	double GetDeterminant() const;

	// returns true if det(M) != 0
	bool IsInvertible() const;


	// the identity matrix
	const static Matrix3D Identity;

	// a matrix where all entries are zero
	const static Matrix3D Zero;

};

bool operator ==(const Matrix3D& m1, const Matrix3D& m2);
bool operator !=(const Matrix3D& m1, const Matrix3D& m2);

Matrix3D operator +(const Matrix3D& m1, const Matrix3D& m2);
Matrix3D operator -(const Matrix3D& m1, const Matrix3D& m2);
Matrix3D operator -(const Matrix3D& m);
Matrix3D operator *(const Matrix3D& m, const double s);
Matrix3D operator *(const double s, const Matrix3D& m);
Matrix3D operator /(const Matrix3D& m, const double s);
Matrix3D operator *(const Matrix3D& m1, const Matrix3D& m2);
Vector3D operator *(const Matrix3D& m, const Vector3D& v);

