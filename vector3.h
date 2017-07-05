#ifndef VECTOR3_H
#define VECTOR3_H
#include <cmath>
#include <algorithm>

#undef min
#undef max

class Vector3
{
public:
	Vector3(float x = 0, float y = 0, float z = 0) :
		x(x), y(y), z(z)
	{
	}

	static Vector3 fromColor(int red, int green, int blue)
	{
		return Vector3(((int)red - 127)/128.f, ((int)green - 127)/128.f, ((int)blue - 127)/128.f);
	}

	float x, y, z;

	int red()   const { return std::max(0, std::min(255, ((int) (x * 128 + 127)))); }
	int green() const { return std::max(0, std::min(255, ((int) (y * 128 + 127)))); }
	int blue()  const { return std::max(0, std::min(255, ((int) (z * 128 + 127)))); }

	float lengthSquared() const
	{
		return x*x + y*y + z*z;
	}

	float length() const
	{
		return std::sqrt(lengthSquared());
	}

	void normalize()
	{
		float l = length();
		if(l)
		{
			x /= l;
			y /= l;
			z /= l;
		}
	}

	float dot(const Vector3 & it) const
	{
		return x*it.x + y*it.y + z*it.z;
	}

	Vector3 cross(const Vector3 & it) const
	{
		return Vector3(y*it.z - z*it.y,
					   z*it.x - x*it.z,
					   x*it.y - y*it.x);
	}

	Vector3 operator*(float m) const
	{
		return Vector3(x*m, y*m, z*m);
	}

	Vector3 operator+(Vector3 v) const
	{
		return Vector3(x+v.x, y+v.y, z+v.z);
	}
};

#endif // VECTOR3_H
