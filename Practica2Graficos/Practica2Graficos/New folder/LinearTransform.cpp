#include "LinearTransform.h"

Matrix *LinearTransform::getScaleTransform(float scaleX, float scaleY, float scaleZ) {
	Matrix *m = new Matrix(4, 4);
	m->setValue(0, 0, scaleX);
	m->setValue(1, 1, scaleY);
	m->setValue(2, 2, scaleZ);
	m->setValue(3, 3, 1);
	return m;
}

Matrix *LinearTransform::getScaleTransform(float scaleX, float scaleY) {
	Matrix *m = new Matrix(3, 3);
	m->setValue(0, 0, scaleX);
	m->setValue(1, 1, scaleY);
	m->setValue(2, 2, 1);
	return m;
}

Matrix *LinearTransform::getTranslateTransform(float transX, float transY, float transZ) {
	Matrix *m = new Matrix(4, 4);
	m->setValue(0, 0, 1);
	m->setValue(1, 1, 1);
	m->setValue(2, 2, 1);
	m->setValue(0, 3, transX);
	m->setValue(1, 3, transY);
	m->setValue(2, 3, transZ);
	m->setValue(3, 3, 1);
	return m;
}

Matrix *LinearTransform::getTranslateTransform(float transX, float transY) {
	Matrix *m = new Matrix(4, 4);
	m->setValue(0, 0, 1);
	m->setValue(1, 1, 1);
	m->setValue(0, 3, transX);
	m->setValue(1, 3, transY);
	m->setValue(2, 2, 1);
	return m;
}

Matrix *LinearTransform::getWindowingTransform(float xl, float yl, float xh, float yh, float xpl, float ypl, float xph, float yph) {
	Matrix *mTrans = getTranslateTransform(xpl, ypl);
	Matrix *mScale = getScaleTransform((xph - xpl) / (xh - xl), (yph - ypl) / (yh - yl));
	Matrix *mTrans2 = getTranslateTransform(-xl, -yl);
	Matrix *res = mTrans->multiply(mScale)->multiply(mTrans2);
	delete mScale;
	delete mTrans;
	delete mTrans2;
	return res;
}

Matrix *LinearTransform::getWindowingTransform(float xl, float yl, float zl, float xh, float yh, float zh, float xpl, float ypl, float zpl, float xph, float yph, float zph) {
	Matrix *mTrans = getTranslateTransform(xpl, ypl, zpl);
	Matrix *mScale = getScaleTransform((xph - xpl) / (xh - xl), (yph - ypl) / (yh - yl), (zph - zpl) / (zh - zl));
	Matrix *mTrans2 = getTranslateTransform(-xl, -yl, -zl);
	Matrix *res = mTrans->multiply(mScale)->multiply(mTrans2);
	delete mScale;
	delete mTrans;
	delete mTrans2;
	return res;
}

Matrix *LinearTransform::getCameraTransform(Point eyePosition, Point gazeVector, Point topVector) {
	Point w = vectorScale(1/vectorMag(gazeVector), gazeVector);
	Point u = vectorScale(1/vectorMag(vectorCross(topVector, w)), vectorCross(topVector, w));
	Point v = vectorCross(w, u);

	Matrix *ma = new Matrix(4, 4);
	ma->setValue(0, 0, u.x);
	ma->setValue(0, 1, u.y);
	ma->setValue(0, 2, u.z);
	ma->setValue(1, 0, v.x);
	ma->setValue(1, 1, v.y);
	ma->setValue(1, 2, v.z);
	ma->setValue(2, 0, w.x);
	ma->setValue(2, 1, w.y);
	ma->setValue(2, 2, w.z);
	ma->setValue(3, 3, 1);

	Matrix *mb = new Matrix(4, 4);
	mb->setValue(0, 0, 1);
	mb->setValue(1, 1, 1);
	mb->setValue(2, 2, 1);
	mb->setValue(3, 3, 1);
	mb->setValue(0, 3, -eyePosition.x);
	mb->setValue(1, 3, -eyePosition.y);
	mb->setValue(2, 3, -eyePosition.x);
	mb->setValue(3, 3, 1);

	Matrix *res = ma->multiply(mb);
	delete ma;
	delete mb;
	return res;
}

Matrix *LinearTransform::getOrthographicTransform(float l, float r, float b, float t, float n, float f) {
	return getWindowingTransform(l, b, n, r, t, f, -1, -1, -1, 1, 1, 1);
}

Matrix *LinearTransform::getViewportTransform(float nx, float ny) {
	return getWindowingTransform(-1, -1, -1, 1, 1, 1, -0.5, -0.5, 1, nx - 0.5, ny - 0.5, 1); 
}

Matrix *LinearTransform::getPerspectiveTransform(float n, float f) {
	Matrix *m = new Matrix(4, 4);
	m->setValue(0, 0, n);
	m->setValue(1, 1, n);
	m->setValue(2, 2, n + f);
	m->setValue(2, 3, -f * n);
	m->setValue(3, 2, 1);
	return m;
}

LinearTransform::LinearTransform(void)
{
}


LinearTransform::~LinearTransform(void)
{
}
