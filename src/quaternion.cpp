#include "quaternion.h"
#include <cmath>

Quaternion::Quaternion(double roll, double pitch, double yaw)
{
	double t0 = std::cos(yaw * 0.5);
	double t1 = std::sin(yaw * 0.5);
	double t2 = std::cos(roll * 0.5);
	double t3 = std::sin(roll * 0.5);
	double t4 = std::cos(pitch * 0.5);
	double t5 = std::sin(pitch * 0.5);

	w = t0 * t2 * t4 + t1 * t3 * t5;
	x = t0 * t3 * t4 - t1 * t2 * t5;
	y = t0 * t2 * t5 + t1 * t3 * t4;
	z = t1 * t2 * t4 - t0 * t3 * t5;
}

Vector3 Quaternion::rotate(const Vector3 & v) const
{
	Vector3 u(x, y, z);
	Vector3 t1 = u * u.dot(v) * 2.f;
	Vector3 t2 = v * (w*w - u.lengthSquared());
	Vector3 t3 = u.cross(v) * w * 2.f;
	return t1+t2+t3;
}
