#ifndef _MATH_LIN_VEC3
#define _MATH_LIN_VEC3

#include <math.h>
#include <string.h>	// memset

namespace SCREEN_Lin {

class SCREEN_Matrix4x4;

// Hm, doesn't belong in this file.
class SCREEN_Vec4 {
public:
	float x,y,z,w;
	SCREEN_Vec4(){}
	SCREEN_Vec4(float a, float b, float c, float d) {x=a;y=b;z=c;w=d;}
};

class SCREEN_Vec3 {
public:
	float x,y,z;

	SCREEN_Vec3() { }
	explicit SCREEN_Vec3(float f) {x=y=z=f;}

	float operator [] (int i) const { return (&x)[i]; }
	float &operator [] (int i) { return (&x)[i]; }

	SCREEN_Vec3(const float _x, const float _y, const float _z) {
		x=_x; y=_y; z=_z;
	}
	void Set(float _x, float _y, float _z) {
		x=_x; y=_y; z=_z;
	}
	SCREEN_Vec3 operator + (const SCREEN_Vec3 &other) const {
		return SCREEN_Vec3(x+other.x, y+other.y, z+other.z);
	}
	void operator += (const SCREEN_Vec3 &other) {
		x+=other.x; y+=other.y; z+=other.z;
	}
	SCREEN_Vec3 operator -(const SCREEN_Vec3 &v) const {
		return SCREEN_Vec3(x-v.x,y-v.y,z-v.z);
	}
	void operator -= (const SCREEN_Vec3 &other)
	{
		x-=other.x; y-=other.y; z-=other.z;
	}
	SCREEN_Vec3 operator -() const {
		return SCREEN_Vec3(-x,-y,-z);
	}

	SCREEN_Vec3 operator * (const float f) const {
		return SCREEN_Vec3(x*f,y*f,z*f);
	}
	SCREEN_Vec3 operator / (const float f) const {
		float invf = (1.0f/f);
		return SCREEN_Vec3(x*invf,y*invf,z*invf);
	}
	void operator /= (const float f)
	{
		*this = *this / f;
	}
	float operator * (const SCREEN_Vec3 &other) const {
		return x*other.x + y*other.y + z*other.z;
	}
	void operator *= (const float f) {
		*this = *this * f;
	}
	void scaleBy(const SCREEN_Vec3 &other) {
		x *= other.x; y *= other.y; z *= other.z;
	}
	SCREEN_Vec3 scaledBy(const SCREEN_Vec3 &other) const {
		return SCREEN_Vec3(x*other.x, y*other.y, z*other.z);
	}
	SCREEN_Vec3 scaledByInv(const SCREEN_Vec3 &other) const {
		return SCREEN_Vec3(x/other.x, y/other.y, z/other.z);
	}
	SCREEN_Vec3 operator *(const SCREEN_Matrix4x4 &m) const;
	void operator *=(const SCREEN_Matrix4x4 &m) {
		*this = *this * m;
	}
	SCREEN_Vec3 rotatedBy(const SCREEN_Matrix4x4 &m) const;
	SCREEN_Vec3 operator %(const SCREEN_Vec3 &v) const {
		return SCREEN_Vec3(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x);
	}	
	float length2() const {
		return x*x + y*y + z*z;
	}
	float length() const {
		return sqrtf(length2());
	}
	void setLength(const float l) {
		(*this) *= l/length();
	}
	SCREEN_Vec3 withLength(const float l) const {
		return (*this) * l / length();
	}
	float distance2To(const SCREEN_Vec3 &other) const {
		return SCREEN_Vec3(other-(*this)).length2();
	}
	SCREEN_Vec3 normalized() const {
		return (*this) / length();
	}
	float normalize() { //returns the previous length, is often useful
		float len = length();
		(*this) = (*this)/len;
		return len;
	}
	bool operator == (const SCREEN_Vec3 &other) const {
		if (x==other.x && y==other.y && z==other.z)
			return true;
		else
			return false;
	}
	SCREEN_Vec3 lerp(const SCREEN_Vec3 &other, const float t) const {
		return (*this)*(1-t) + other*t;
	}
	void setZero() {
		memset((void *)this,0,sizeof(float)*3);
	}
};

inline SCREEN_Vec3 operator * (const float f, const SCREEN_Vec3 &v) {return v * f;}

// In new code, prefer these to the operators.

inline float dot(const SCREEN_Vec3 &a, const SCREEN_Vec3 &b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline SCREEN_Vec3 cross(const SCREEN_Vec3 &a, const SCREEN_Vec3 &b) {
	return a % b;
}

class SCREEN_AABBox {
public:
	SCREEN_Vec3 min;
	SCREEN_Vec3 max;
};

}  // namespace SCREEN_Lin

#endif	// _MATH_LIN_VEC3
