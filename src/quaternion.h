#ifndef QUATERNION_H
#define QUATERNION_H
#include "vector3.h"

class Quaternion
{
public:
	Quaternion(double roll, double pitch, double yaw);
	Quaternion(Vector3 axis, double angle);

	double w, x, y, z;
	Vector3 rotate(const Vector3 & v) const;
};

#endif // QUATERNION_H
