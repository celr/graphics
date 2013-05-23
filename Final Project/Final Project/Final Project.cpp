// Practica2Graficos.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

/*
 * Line drawing algorithm.
 *
 * This program reads  two pairs of  integers  from standard input,
 * each one representing  the start and  end coordinates  (x, y) of
 * a line. Then it prints to standard output a list of coordinates:
 * the pixels to be drawn in order to draw the line.
 *
 * Authors:
 *   + Carlos E. Lopez Rivas
 *   + S. Adonais Romero Gonzalez
 */

#include "Final Project.h"

typedef unsigned char uchar;

typedef struct
{
	uchar r;
	uchar g;
	uchar b;
} Color;

const Color COLOR_BLACK = {0, 0, 0};
const Color COLOR_WHITE = {255, 255, 255};

typedef struct
{
	Color **col;
	int w;
	int h;
	double **z;
} Canvas;

int abs(int a);
void swap(int *, int *);

Canvas *createCanvas(int, int);
void canvasToPPM(Canvas *, const char *);
void drawPixel(Canvas *, int, int, Color);
void drawPixelZ(Canvas *, int, int, double, Color);
void drawLine(Canvas *, int, int, int, int, Color);
void drawTriangle(Canvas *, int, int, double, int, int, double, int, int, double, Color);


Matrix *matrixFromRaw(char *fileName, vector<int> *lineEndings) {
	ifstream file;
	file.open(fileName);
	Matrix *m = new Matrix(4, 0);

	while(!file.eof()) {
		float x, y, z;
		string line;
		stringstream sline;
		getline(file, line);
		sline.str(line);

		while(!sline.eof()) {
			sline >> x >> y >> z;
			m->addColumn();
			m->setValue(0, m->cols - 1, x);
			m->setValue(1, m->cols - 1, y);
			m->setValue(2, m->cols - 1, z);
			m->setValue(3, m->cols - 1, 1);
		}

		// Guardar fines de lineas para separar caras
		lineEndings->push_back(m->cols - 1);
	}
	
	return m;
}

void swap(int *a, int *b) {
	int c;
	c = *a;
	*a = *b;
	*b = c;
}

void swapDouble(double *a, double *b) {
	double c;
	c = *a;
	*a = *b;
	*b = c;
}

void drawPixelZ(Canvas *canvas, int x, int y, double z, Color c) {
	if (x < 0 || y < 0 || x >= canvas->w || y >= canvas->h)
		return;
	if (z > canvas->z[x][y] || z < 0)
		return;
	canvas->z[x][y] = z;
	canvas->col[x][y] = c;
}

void drawPixel(Canvas *canvas, int x, int y, Color c) {
	if (x < 0 || y < 0 || x >= canvas->w || y >= canvas->h)
		return;
	canvas->col[x][y] = c;
}

void drawLine(Canvas *canvas, int x_1, int y_1, int x_2, int y_2, Color c) {
	int sx, sy;
	int err, e2;
	int dx, dy;
	dx = abs(x_2 - x_1);
	dy = abs(y_2 - y_1);
	sx = (x_1 < x_2) ? 1 : -1;
	sy = (y_1 < y_2) ? 1 : -1;
	err = dx - dy;
	drawPixel(canvas, x_1, y_1, c);
	while (x_1 != x_2 || y_1 != y_2) {
		e2 = 2 * err;
		if (e2 > -dy) { 
			err -= dy;
			x_1 += sx;
		}
		if (e2 < dx) { 
			err += dx;
			y_1 += sy; 
		}
		drawPixel(canvas, x_1, y_1, c);
	}
}

void drawScanline(Canvas *canvas, int x_1, int x_2, int y, double z_1, double z_2, Color c)
{
	double dz, mz;
	if (x_1 > x_2)
	{
		swap(&x_1, &x_2);
		swapDouble(&z_1, &z_2);
	}
	dz = (z_2 - z_1) / (x_2 - x_1);
	mz = z_1;
	while (x_1 <= x_2)
	{
		drawPixelZ(canvas, x_1, y, mz, c);
		x_1++;
		mz += dz;
	}
}

void drawTriangle(Canvas *canvas, int x_1, int y_1, double z_1, int x_2, int y_2, double z_2, int x_3, int y_3, double z_3, Color c)
{
	int dx_1, dy_1, dx_2, dy_2, sx_1, sx_2;
	double dz_1, dz_2;
	int mx_1, mx_2, y;
	double mz_1, mz_2;
	int err_1, err_2;
	if (y_2 < y_1)
	{
		swap(&x_1, &x_2);
		swap(&y_1, &y_2);
		swapDouble(&z_1, &z_2);
	}
	if (y_3 < y_1)
	{
		swap(&x_1, &x_3);
		swap(&y_1, &y_3);
		swapDouble(&z_1, &z_3);
	}
	if (y_3 < y_2)
	{
		swap(&x_2, &x_3);
		swap(&y_2, &y_3);
		swapDouble(&z_2, &z_3);
	}
	dx_2 = abs(x_3 - x_1);
	sx_2 = (x_3 > x_1) ? 1 : -1;
	dy_2 = y_3 - y_1;
	if (y_1 == y_2)
	{
		if (y_2 == y_3)
		{
			drawScanline(canvas, x_1, x_2, y_1, z_1, z_2, c);
			drawScanline(canvas, x_2, x_3, y_1, z_2, z_3, c);
			return;
		}
		drawScanline(canvas, x_1, x_2, y_1, z_1, z_2, c);
		err_2 = dx_2;
		mx_2 = x_1 + sx_2 * (err_2 / dy_2);
		err_2 = err_2 % dy_2;
		dz_2 = (z_3 - z_1) / dy_2; 
		mz_2 = z_1 + dz_2;
	}
	else
	{
		dx_1 = abs(x_2 - x_1);
		sx_1 = (x_2 > x_1) ? 1 : -1;
		dy_1 = y_2 - y_1;
		y = y_1;
		mx_1 = mx_2 = x_1;
		err_1 = err_2 = 0;
		dz_1 = (z_2 - z_1) / dy_1;
		dz_2 = (z_3 - z_1) / dy_2;
		mz_1 = mz_2 = z_1;
		while (y <= y_2)
		{
			drawScanline(canvas, mx_1, mx_2, y, mz_1, mz_2, c);
			y++;
			err_1 += dx_1;
			err_2 += dx_2;
			mx_1 += sx_1 * (err_1 / dy_1);
			mx_2 += sx_2 * (err_2 / dy_2);
			err_1 = err_1 % dy_1;
			err_2 = err_2 % dy_2;
			mz_1 += dz_1;
			mz_2 += dz_2;
		}
	}
	if (y_2 != y_3)
	{
		dx_1 = abs(x_3 - x_2);
		dy_1 = y_3 - y_2;
		sx_1 = (x_3 > x_2) ? 1 : -1;
		y = y_2 + 1;
		mx_1 = x_2 + sx_1 * (dx_1 / dy_1);
		err_1 = dx_1 % dy_1;
		dz_1 = (z_3 - z_2) / dy_1;
		mz_1 = z_2 + dz_1;
		while(y <= y_3)
		{
			drawScanline(canvas, mx_1, mx_2, y, mz_1, mz_2, c);
			y++;
			err_1 += dx_1;
			err_2 += dx_2;
			mx_1 += sx_1 * (err_1 / dy_1);
			mx_2 += sx_2 * (err_2 / dy_2);
			err_1 = err_1 % dy_1;
			err_2 = err_2 % dy_2;
			mz_1 += dz_1;
			mz_2 += dz_2;
		}
	}
}

Canvas *createCanvas(int w, int h) {
	int i, j;
	Canvas *c;
	double **z;
	Color **col;
	col = new Color*[w];
	z = new double*[w];
	for (i = 0; i < w; i++) {
		col[i] = new Color[h];
		z[i] = new double[w];
		for (j = 0; j < h; j++) {
			col[i][j] = COLOR_BLACK;
			z[i][j] = numeric_limits<double>::max();
		}
	}
	c = new Canvas;
	c->col = col;
	c->w = w;
	c->h = h;
	c->z = z;
	return c;
}

void canvasToPPM(Canvas *canvas, const char *filename) {
	FILE *out;
	int i, j;
	out = fopen(filename, "w");
	fprintf(out, "P6\n%d %d\n%d\n", canvas->w, canvas->h, 255);
	for (j = canvas->h - 1; j >= 0; j--)
		for (i = 0; i < canvas->w; i++)
		{
			fwrite(&canvas->col[i][j].r, 1, 1, out);
			fwrite(&canvas->col[i][j].g, 1, 1, out);
			fwrite(&canvas->col[i][j].b, 1, 1, out);
		}
	fclose(out);
}

Point pointDiff(Matrix *a, Matrix *b)
{
	Point c;
	c.x = a->getValue(0, 0) - b->getValue(0, 0);
	c.y = a->getValue(1, 0) - b->getValue(1, 0);
	c.z = a->getValue(2, 0) - b->getValue(2, 0);
	return c;
}

void drawTriangleFromPoints(Canvas *canvas, Matrix *point1, Matrix *point2, Matrix *point3, Color color) {
	drawTriangle(canvas, point1->getValue(0, 0) / point1->getValue(3, 0), 
				point1->getValue(1, 0) / point1->getValue(3, 0), 
				point1->getValue(2, 0) / point1->getValue(3, 0), 
				point2->getValue(0, 0) / point2->getValue(3, 0),
				point2->getValue(1, 0) / point2->getValue(3, 0),
				point2->getValue(2, 0) / point2->getValue(3, 0),
				point3->getValue(0, 0) / point3->getValue(3, 0),
				point3->getValue(1, 0) / point3->getValue(3, 0),
				point3->getValue(2, 0) / point3->getValue(3, 0), color);
}

void drawMatrix(Canvas *canvas, Matrix *matrix, Matrix *transform, vector<int> *lineEndings) {
	vector<Matrix *> currentFace(0);
	for (int i = 0, k = 0; i < matrix->cols; i++) {
		// Create a small matrix for the current column
		Matrix *point = new Matrix(4, 1);
		point->setValue(0, 0, matrix->getValue(0, i));
		point->setValue(1, 0, matrix->getValue(1, i));
		point->setValue(2, 0, matrix->getValue(2, i));
		point->setValue(3, 0, 1);

		// Apply the supplied transform to the point
		Matrix *transformedPoint = transform->multiply(point);

		if (i > 0 && i % 3 == 0) {
			// Create triangle for face
			Matrix *point1 = currentFace[0];
			Matrix *point2 = currentFace[1];
			Matrix *point3 = currentFace[2];
			drawTriangleFromPoints(canvas, point1, point2, point3, COLOR_WHITE);
			currentFace.clear();
		}

		currentFace.push_back(transformedPoint);
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	int w, h;
	w = 1000;
	h = 1000;
	float d = 0.2; // Distancia entre ojos

	vector<int> lineEndings;
	Matrix *mx = matrixFromRaw("in.raw", &lineEndings);
	Matrix *mx2 = LinearTransform::getViewportTransform(w, h);
	Matrix *mx3 = mx2->multiply(LinearTransform::getOrthographicTransform(-5, 5, -5, 5, -5, 5));
	Matrix *mx4 = mx3->multiply(LinearTransform::getPerspectiveTransform(-2, 2));

	Point eye;
	eye.x = 0;
	eye.y = 2;
	eye.z = 3;

	Point gaze;
	gaze.x = 0;
	gaze.y = 10;
	gaze.z = 3;

	Point top;
	top.x = 0;
	top.y = 1;
	top.z = 0;

	Matrix *mx5 = mx4->multiply(LinearTransform::getCameraTransform(eye, gaze, top));

	Canvas *canvas = createCanvas(w, h);
	drawMatrix(canvas, mx, mx5, &lineEndings);
	canvasToPPM(canvas, "res.ppm");
}