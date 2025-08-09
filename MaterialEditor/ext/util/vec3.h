#pragma once
#include <cmath>

class Vec3
{
public:
	float x, y, z;

public:
	inline constexpr Vec3() : x(0.f), y(0.f), z(0.f) { }
	inline constexpr Vec3(float x, float y, float z) : x(x), y(y), z(z) { }
	inline constexpr Vec3(float v) : x(v), y(v), z(v) { }
	inline constexpr Vec3(float* v) : x(v[0]), y(v[1]), z(v[2]) { }

	// Vec3 operators
	inline Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
	inline Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
	inline Vec3 operator*(const Vec3& v) const { return Vec3(x * v.x, y * v.y, z * v.z); }
	inline Vec3 operator/(const Vec3& v) const { return Vec3(x / v.x, y / v.y, z / v.z); }

	inline Vec3& operator+=(const Vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
	inline Vec3& operator-=(const Vec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
	inline Vec3& operator*=(const Vec3& v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
	inline Vec3& operator/=(const Vec3& v) { x /= v.x; y /= v.y; z /= v.z; return *this; }

	inline Vec3 operator-() const { return Vec3(-x, -y, -z); }

	inline bool operator==(const Vec3& v) const { return x == v.x && y == v.y && z == v.z; }
	inline bool operator!=(const Vec3& v) const { return !(*this == v); }

	// Float operators
	inline Vec3 operator+(float v) const { return Vec3(x + v, y + v, z + v); }
	inline Vec3 operator-(float v) const { return Vec3(x - v, y - v, z - v); }
	inline Vec3 operator*(float v) const { return Vec3(x * v, y * v, z * v); }
	inline Vec3 operator/(float v) const { float inv = 1.f / v; return Vec3(x * inv, y * inv, z * inv); }

	inline Vec3& operator+=(float v) { x += v; y += v; z += v; return *this; }
	inline Vec3& operator-=(float v) { x -= v; y -= v; z -= v; return *this; }
	inline Vec3& operator*=(float v) { x *= v; y *= v; z *= v; return *this; }
	inline Vec3& operator/=(float v) { float inv = 1.f / v; x *= inv; y *= inv; z *= inv; return *this; }

	inline bool operator==(float v) const { return x == v && y == v && z == v; }
	inline bool operator!=(float v) const { return !(*this == v); }

	inline float& operator[](int i) const { assert(i >= 0 && i < 3); return ((float*)&x)[i]; }

	// Utility functions
	inline bool  isZero()             const { return *this == 0.f; }
	inline float length()             const { return sqrtf(x * x + y * y + z * z); }
	inline float length2d()           const { return sqrtf(x * x + y * y); }
	inline Vec3  normalize()          const { float l = length(); float inv = 1.f / l; return Vec3(x * inv, y * inv, z * inv); }
	inline float dot(const Vec3& v)   const { return x * v.x + y * v.y + z * v.z; }
	inline Vec3  cross(const Vec3& v) const { return Vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }
	inline Vec3  lerp(const Vec3& v, float t) const { return *this + (v - *this) * std::min(t, 1.f); }

	// Other functions
	inline float distanceTo(const Vec3& v) const
	{
		return (v - *this).length();
	}

	inline Vec3 rotateVec(const Vec3& v) const
	{
		return *this * v * -(*this);
	}
	inline Vec3 rotate(const Vec3& rot, float w)
	{
		Vec3 t = rot.cross(*this);   // no *2 yet
		t *= 2.0f;                  // 2 × (qv × v)

		return *this
			+ t * w               // 2 qw (qv × v)
			+ rot.cross(t);         // 2 (qv × (qv × v))
	}
};