#include "StdAfx.h"
#include "Vector3DF.h"


Vector3DF::Vector3DF()
{

}
Vector3DF::Vector3DF(const float _x, float _y, float _z)
{
	x = _x;
	y = _y;
	z = _z;
}
Vector3DF::~Vector3DF()
{
}

Vector3DF& Vector3DF::operator= (const Vector3DF& p)
{
	x = p.x;
	y = p.y;
	z = p.z;
	return *this;
}
Vector3DF Vector3DF::operator+ (const Vector3DF& p)
{
	Vector3DF sum;
	sum.x = x+p.x;
	sum.y = y+p.y;
	sum.z = z+p.z;
	return sum;
}
Vector3DF Vector3DF::operator- (const Vector3DF& p)
{
	Vector3DF sum;
	sum.x = x-p.x;
	sum.y = y-p.y;
	sum.z = z-p.z;
	return sum;
}
Vector3DF& Vector3DF::operator+= (const Vector3DF& p)
{
	x += p.x;
	y += p.y;
	z += p.z;
	return *this;
}
Vector3DF& Vector3DF::operator-= (const Vector3DF& p)
{
	x -= p.x;
	y -= p.y;
	z -= p.z;
	return *this;
}

void Vector3DF::Set(const float _x, const float _y, const float _z)
{
	x = _x;
	y = _y;
	z = _z;
}

Vector3DF Vector3DF::operator*(const float p)
{
	Vector3DF result;
	result.x = this->x * p;
	result.y = this->y * p;
	result.z = this->z * p;
	return result;
}

//µã³Ë
float Vector3DF::operator* (const Vector3DF& p)
{
	float result = x*p.x + y*p.y + z*p.z;
	return result;
}

float Vector3DF::operator[](const int i)
{
	switch(i)
	{
	case 0:
		return x;
		break;
	case 1:
		return y;
		break;
	case 2:
		return z;
		break;
	default:
		return 0;
		break;
	}
}

float Vector3DF::DistanceTo(const Vector3DF p)
{
	float dis = sqrt((x-p.x)*(x-p.x) + (y-p.y)*(y-p.y) + (z-p.z)*(z-p.z));
	return dis;
}


void Vector3DF::Normal()
{
	float dis = sqrt(x*x + y*y + z*z);
	x = x/dis;
	y = y/dis;
	z = z/dis;
}