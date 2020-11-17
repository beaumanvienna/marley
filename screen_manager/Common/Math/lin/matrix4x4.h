#ifndef _MATH_LIN_MATRIX4X4_H
#define _MATH_LIN_MATRIX4X4_H

#include "Common/Math/lin/vec3.h"

namespace SCREEN_Lin {

class SCREEN_Matrix4x4 {
public:
	union {
		struct {
			float xx, xy, xz, xw;
			float yx, yy, yz, yw;
			float zx, zy, zz, zw;
			float wx, wy, wz, ww;
		};
		float m[16];
	};

	const SCREEN_Vec3 right() const {return SCREEN_Vec3(xx, xy, xz);}
	const SCREEN_Vec3 up()		const {return SCREEN_Vec3(yx, yy, yz);}
	const SCREEN_Vec3 front() const {return SCREEN_Vec3(zx, zy, zz);}
	const SCREEN_Vec3 move()	const {return SCREEN_Vec3(wx, wy, wz);}

	const float &operator[](int i) const {
		return *(((const float *)this) + i);
	}
	float &operator[](int i) {
		return *(((float *)this) + i);
	}
	SCREEN_Matrix4x4 operator * (const SCREEN_Matrix4x4 &other) const ;
	void operator *= (const SCREEN_Matrix4x4 &other) {
		*this = *this * other;
	}
	const float *getReadPtr() const {
		return (const float *)this;
	}
	void empty() {
		memset(this, 0, 16 * sizeof(float));
	}
	static SCREEN_Matrix4x4 identity() {
		SCREEN_Matrix4x4 id;
		id.setIdentity();
		return id;
	}
	void setIdentity() {
		empty();
		xx = yy = zz = ww = 1.0f;
	}
	void setTranslation(const SCREEN_Vec3 &trans) {
		setIdentity();
		wx = trans.x;
		wy = trans.y;
		wz = trans.z;
	}

	SCREEN_Matrix4x4 transpose() const;

	// Exact angles to avoid any artifacts.
	void setRotationZ90() {
		empty();
		float c = 0.0f;
		float s = 1.0f;
		xx = c; xy = s;
		yx = -s; yy = c;
		zz = 1.0f;
		ww = 1.0f;
	}
	void setRotationZ180() {
		empty();
		float c = -1.0f;
		float s = 0.0f;
		xx = c; xy = s;
		yx = -s; yy = c;
		zz = 1.0f;
		ww = 1.0f;
	}
	void setRotationZ270() {
		empty();
		float c = 0.0f;
		float s = -1.0f;
		xx = c; xy = s;
		yx = -s; yy = c;
		zz = 1.0f;
		ww = 1.0f;
	}

	void setOrtho(float left, float right, float bottom, float top, float near, float far);
	void setOrthoD3D(float left, float right, float bottom, float top, float near, float far);
	void setOrthoVulkan(float left, float right, float top, float bottom, float near, float far);

	void setViewFrame(const SCREEN_Vec3 &pos, const SCREEN_Vec3 &right, const SCREEN_Vec3 &forward, const SCREEN_Vec3 &up);
	void toText(char *buffer, int len) const;
	void print() const;

	void translateAndScale(const SCREEN_Vec3 &trans, const SCREEN_Vec3 &scale) {
		xx = xx * scale.x + xw * trans.x;
		xy = xy * scale.y + xw * trans.y;
		xz = xz * scale.z + xw * trans.z;

		yx = yx * scale.x + yw * trans.x;
		yy = yy * scale.y + yw * trans.y;
		yz = yz * scale.z + yw * trans.z;

		zx = zx * scale.x + zw * trans.x;
		zy = zy * scale.y + zw * trans.y;
		zz = zz * scale.z + zw * trans.z;

		wx = wx * scale.x + ww * trans.x;
		wy = wy * scale.y + ww * trans.y;
		wz = wz * scale.z + ww * trans.z;
	}
};

}  // namespace SCREEN_Lin

#endif	// _MATH_LIN_MATRIX4X4_H

