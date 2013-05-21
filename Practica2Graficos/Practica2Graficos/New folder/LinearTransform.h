#pragma once
#include "Matrix.h"
#include "Point.h"

// Contains a basic set of matrix linear transforms both in 2D and 3D.
class LinearTransform
{
public:
	static Matrix *getScaleTransform(float scaleX, float scaleY, float scaleZ);
	static Matrix *getScaleTransform(float scaleX, float scaleY);
	static Matrix *getTranslateTransform(float transX, float transY, float transZ);
	static Matrix *getTranslateTransform(float transX, float transY);
	static Matrix *getWindowingTransform(float xl, float yl, float xh, float yh, float xpl, float ypl, float xph, float yph);
	static Matrix *getWindowingTransform(float xl, float yl, float zl, float xh, float yh, float zh, float xpl, float ypl, float zpl, float xph, float yph, float zph);
	static Matrix *getCameraTransform(Point eyePosition, Point gazeVector, Point topVector);
	static Matrix *getOrthographicTransform(float l, float r, float b, float t, float n, float f);
	static Matrix *getViewportTransform(float nx, float ny);
	static Matrix *getPerspectiveTransform(float n, float f);
	LinearTransform(void);
	~LinearTransform(void);
};
