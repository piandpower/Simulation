#include "Vector3D.h"

Vector3D::Vector3D() : X(0), Y(0), Z(0)
{
}

Vector3D::Vector3D(const double x, const double y, const double z) : X(x), Y(y), Z(z)
{
}

Vector3D::Vector3D(const float x, const float y, const float z) : X(x), Y(y), Z(z)
{
}

Vector3D::Vector3D(const int x, const int y, const int z) : X(x), Y(y), Z(z)
{
}

Vector3D::Vector3D(const double s) : X(s), Y(s), Z(s)
{
}

Vector3D::Vector3D(const FVector * const v) : X(v->X), Y(v->Y), Z(v->Z)
{
}

Vector3D::Vector3D(FVector v) : X(v.X), Y(v.Y), Z(v.Z)
{
}

Vector3D::operator FVector()
{
	return FVector(X, Y, Z);
}

Vector3D::~Vector3D()
{
}

const Vector3D Vector3D::Zero = { 0.0, 0.0, 0.0 };
const Vector3D Vector3D::One = { 1.0, 1.0, 1.0 };

void Vector3D::operator+=(const Vector3D & v)
{
	X += v.X;
	Y += v.Y;
	Z += v.Z;
}

void Vector3D::operator-=(const Vector3D & v)
{
	X -= v.X;
	Y -= v.Y;
	Z -= v.Z;
}

void Vector3D::operator*=(const double s)
{
	X *= s;
	Y *= s;
	Z *= s;
}

void Vector3D::operator/=(const double s)
{
	X /= s;
	Y /= s;
	Z /= s;
}

void Vector3D::operator*=(const Vector3D & v)
{
	X *= v.X;
	Y *= v.Y;
	Z *= v.Z;
}

void Vector3D::operator/=(const Vector3D & v)
{
	X /= v.X;
	Y /= v.Y;
	Z /= v.Z;
}

void Vector3D::operator%=(const Vector3D & v)
{
	Vector3D temp(Y * v.Z - Z * v.Y, Z * v.X - X * v.Z, X * v.Y - Y * v.X);
	*this = temp;
}

void Vector3D::operator=(const Vector3D & v)
{
	X = v.X;
	Y = v.Y;
	Z = v.Z;
}

void Vector3D::operator=(const FVector & v)
{
	X = v.X;
	Y = v.Y;
	Z = v.Z;
}

void Vector3D::Negate()
{
	X = -X;
	Y = -Y;
	Z = -Z;
}

double Vector3D::Normalize()
{
	double length = Length();

	// check if vector isn't zero vector
	if (length == 0) {
		return 0.0;
	}

	X /= length;
	Y /= length;
	Z /= length;

	return 1 / length;
}

Vector3D Vector3D::Normalized() const
{
	double length = Length();

	// check if vector isn't zero vector
	if (length == 0) {
		return Vector3D(0.0);
	}

	return Vector3D( X / length, Y / length, Z / length);
}

double Vector3D::Length() const
{
	return sqrt(X * X + Y * Y + Z * Z);
}

double Vector3D::Size() const
{
	return Length();
}

double Vector3D::LengthSquared() const
{
	return X * X + Y * Y + Z * Z;
}

bool Vector3D::CloseTo(const Vector3D & v, double tolerance)
{
	if (X - v.X >= tolerance || v.X - X >= tolerance) {
		return false;
	}
	if (Y - v.Y >= tolerance || v.Y - Y >= tolerance) {
		return false;
	}
	if (Z - v.Z >= tolerance || v.Z - Z >= tolerance) {
		return false;
	}
	return true;
}

Vector3D Vector3D::CrossProduct(const Vector3D & v1, const Vector3D & v2)
{
	return Vector3D(v1.Y * v2.Z - v1.Z * v2.Y, v1.Z * v2.X - v1.X * v2.Z, v1.X * v2.Y - v1.Y * v2.X);
}

Vector3D Vector3D::ComponentwiseMultiplication(const Vector3D & v1, const Vector3D & v2)
{
	return Vector3D(v1.X * v2.X, v1.Y * v2.Y, v1.Z * v2.Z);
}

Vector3D Vector3D::ProjectVector(const Vector3D & receiver, const Vector3D & shadower)
{
	return (shadower * receiver) / receiver.LengthSquared() * receiver;
}

double Vector3D::ProjectScalar(const Vector3D & receiver, const Vector3D & shadower)
{
	return (shadower * receiver) / receiver.LengthSquared();
}

std::string Vector3D::ToString() const
{
	std::ostringstream x;
	x << std::setprecision(std::numeric_limits<double>::digits10 + 1) << X;
	std::ostringstream y;
	y << std::setprecision(std::numeric_limits<double>::digits10 + 1) << Y;
	std::ostringstream z;
	z << std::setprecision(std::numeric_limits<double>::digits10 + 1) << Z;
	return "{ X: " + x.str() + ", Y: " + y.str() + ", Z: " + z.str() + " }";
}

Vector3D Vector3D::FromString(std::string string)
{
	Vector3D vector;

	int start = string.find("X: ");
	int end = string.find(",");
	vector.X = std::stod(string.substr(start + 3, end - start - 3));

	start = string.find("Y: ");
	end = string.find(",", start);
	vector.Y = std::stod(string.substr(start + 3, end - start - 3));

	start = string.find("Z: ");
	end = string.find(",", start);
	vector.Z = std::stod(string.substr(start + 3, end - start - 3));

	return vector;
}

std::ostream & operator<<(std::ostream & stream, const Vector3D & v)
{
	stream << v.ToString();
	return stream;
}

bool operator==(const Vector3D & v1, const Vector3D & v2)
{
	return v1.X == v2.X && v1.Y == v2.Y, v1.Z == v2.Z;
}

bool operator!=(const Vector3D & v1, const Vector3D & v2)
{
	return !(v1 == v2);
}

bool operator<(const Vector3D & v1, const Vector3D & v2)
{
	return v1.Length() < v2.Length();
}

bool operator>(const Vector3D & v1, const Vector3D & v2)
{
	return v2 < v1;
}

bool operator<=(const Vector3D & v1, const Vector3D & v2)
{
	return !(v1 > v2);
}

bool operator>=(const Vector3D & v1, const Vector3D & v2)
{
	return !(v1 < v2);
}

Vector3D operator+(const Vector3D & v1, const Vector3D & v2)
{
	return Vector3D(v1.X + v2.X, v1.Y + v2.Y, v1.Z + v2.Z);
}

Vector3D operator-(const Vector3D & v1, const Vector3D & v2)
{
	return Vector3D(v1.X - v2.X, v1.Y - v2.Y, v1.Z - v2.Z);
}

Vector3D operator-(const Vector3D & v)
{
	return Vector3D(-v.X, -v.Y, -v.Z);
}

Vector3D operator%(const Vector3D & v1, const Vector3D & v2)
{
	return Vector3D::CrossProduct(v1, v2);
}

Vector3D operator*(const Vector3D & v, const double d)
{
	return Vector3D(v.X * d, v.Y * d, v.Z * d);
}

Vector3D operator*(const double d, const Vector3D & v)
{
	return Vector3D(v.X * d, v.Y * d, v.Z * d);
}

Vector3D operator/(const Vector3D & v, const double d)
{
	return Vector3D(v.X / d, v.Y / d, v.Z / d);
}

double operator*(const Vector3D & v1, const Vector3D & v2)
{
	return v1.X * v2.X + v1.Y * v2.Y + v1.Z * v2.Z;
}
