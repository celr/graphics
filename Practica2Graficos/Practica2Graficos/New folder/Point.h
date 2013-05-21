#pragma once
#include <vector>
using namespace std;
// Represents a point
struct RawPoint {
	float x;
	float y;
	float z;
};

typedef struct RawPoint RawPoint;
typedef vector<RawPoint> RawFace;
typedef vector<RawFace> RawMap;
typedef RawPoint Point;

// Basic vector operations
Point vectorScale(float scale, Point v);
float vectorMag(Point v);
Point vectorCross(Point a, Point b);