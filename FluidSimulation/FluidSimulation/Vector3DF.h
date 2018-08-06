#pragma once
#include <math.h>
class Vector3DF
{
public:
	float x,y,z;
	Vector3DF();
	Vector3DF(const float _x, float _y, float _z);
	~Vector3DF();

	Vector3DF& operator= (const Vector3DF& p);
	Vector3DF operator+ (const Vector3DF& p);
	Vector3DF operator- (const Vector3DF& p);
	Vector3DF& operator+= (const Vector3DF& p);
	Vector3DF& operator-= (const Vector3DF& p);

	Vector3DF operator* (const float p);
	float operator* (const Vector3DF& p); //点乘
	float operator[] (const int i);

	void Set(const float _x, const float _y, const float _z);
	float DistanceTo(const Vector3DF p);
	void Normal();  //单位化

};

