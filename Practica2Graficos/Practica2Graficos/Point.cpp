#include "stdafx.h"
#include "Point.h"

Point vectorScale(float scale, Point v) {
	Point p;
	p.x = scale * v.x;
	p.y = scale * v.y;
	p.z = scale * v.z;
	return p;
}

float vectorMag(Point v) {
	return sqrt(pow(v.x, 2) + pow(v.y, 2) + pow(v.z, 2));
}

Point vectorCross(Point a, Point b) {
	Point p;
	p.x = (a.y * b.z) - (a.z * b.y);
	p.y = (a.z * b.x) - (a.x * b.z);
	p.z = (a.x * b.y) - (a.y * b.x);
	return p;
}