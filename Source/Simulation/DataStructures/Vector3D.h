#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include <math.h>
#include "CoreMinimal.h"

class Vector3D {

public:
	// X component of Vector
	double X;
	
	// Y component of Vector
	double Y;

	// Z component of Vector
	double Z;

	// constructor/destructor
	Vector3D();
	Vector3D(const double x, const double y, const double z);
	Vector3D(const float x, const float y, const float z);
	Vector3D(const int x, const int y, const int z);
	Vector3D(const FVector * const v);
	explicit Vector3D(const FVector v);
	explicit Vector3D(const double s);

	// Conversion to FVector
	operator FVector();

	~Vector3D();

	const static Vector3D Zero;
	const static Vector3D One;

	void operator +=(const Vector3D& v);
	void operator -=(const Vector3D& v);
	void operator *=(const double s);
	void operator /=(const double s);
	void operator *=(const Vector3D& v);
	void operator /=(const Vector3D& v);
	void operator %=(const Vector3D& v);
	
	void operator =(const Vector3D& v);
	void operator =(const FVector& v);

	void Negate();

	// Normalizes the vector in place and returns the factor it has been multiplied with
	double Normalize();

	// Returns the normalized vector
	Vector3D Normalized() const;

	double Length() const;
	double Size() const;
	double LengthSquared() const;

	bool CloseTo(const Vector3D& v, double tolerance = 0.001);

	static Vector3D CrossProduct(const Vector3D& v1, const Vector3D& v2);
	static Vector3D ComponentwiseMultiplication(const Vector3D & v1, const Vector3D& v2);
	static Vector3D ProjectVector(const Vector3D& receiver, const Vector3D& shadower);
	static double ProjectScalar(const Vector3D& receiver, const Vector3D& shadower);

	std::string ToString() const;

	static Vector3D FromString(std::string string);
};

// Returns true if all Coordinates are the same 
bool operator ==(const Vector3D& v1, const Vector3D& v2);

// Returns true if at least one Coordinate differs
bool operator !=(const Vector3D& v1, const Vector3D& v2);

// Compares the Length of two Vectors
bool operator <(const Vector3D& v1, const Vector3D& v2);

// Compares the Length of two Vectors
bool operator >(const Vector3D& v1, const Vector3D& v2);

// Compares the Length of two Vectors

bool operator <=(const Vector3D& v1, const Vector3D& v2);

// Compares the Length of two Vectors
bool operator >=(const Vector3D& v1, const Vector3D& v2);

// Adds two Vectors
Vector3D operator +(const Vector3D& v1, const Vector3D& v2);

// Difference between two Vectors
Vector3D operator -(const Vector3D& v1, const Vector3D& v2);

// Negated Vector
Vector3D operator -(const Vector3D& v);

// Cross Product
Vector3D operator %(const Vector3D& v1, const Vector3D& v2);

// Vector Scaled
Vector3D operator *(const Vector3D& v, const double d);

// Vector Scaled
Vector3D operator *(const double d, const Vector3D& v);

// Vector Scaled
Vector3D operator /(const Vector3D& v, const double d);

// Inner Product
double operator *(const Vector3D& v1, const Vector3D& v2);