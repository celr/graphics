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

#include "Practica2Graficos.h"
int abs(int a);
void swap(int *, int *);

char ***createCanvas(int, int);
void canvasToPPM(char ***, int, int, char *);
void orderPairs(int *, int *, int *, int *);
void drawPixel(char ***, int, int);
void drawLine(char ***, int, int, int, int, int, int, int);

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

int abs(int a) {
	return (a > 0) ? a : -a;
}

void swap(int *a, int *b) {
	int c;
	c = *a;
	*a = *b;
	*b = c;
}

void orderPairs(int *x_1, int *y_1, int *x_2, int *y_2) {
	if (*x_1 > *x_2) {
		swap(x_1, x_2);
		swap(y_1, y_2);
	}
}

void drawPixel(char ***canvas, int x, int y, int r, int g, int b) {
	int k;
	if (x < 0 || y < 0 || x >= 1920 || y >= 1080)
		return;
	canvas[x][y][0] = g;
	canvas[x][y][1] = b;
	canvas[x][y][2] = r;
}

void drawLine(char ***canvas, int x_1, int y_1, int x_2, int y_2, int r, int g, int b) {
	int sx, sy;
	int err, e2;
	int dx, dy;
	dx = abs(x_2-x_1);
	dy = abs(y_2-y_1); 
	sx = (x_1 < x_2) ? 1  : -1;
	sy = (y_1 < y_2) ? 1  : -1;
	err = dx - dy;
	drawPixel(canvas, x_1, y_1, r, g, b);
	while (x_1 != x_2 || y_1 != y_2) {
		e2 = 2 * err;
		if (e2 > -dy) { 
			err -= dy;
			x_1 += sx;
		}
		if (e2 <  dx) { 
			err += dx;
			y_1 +=  sy; 
		}
		drawPixel(canvas, x_1, y_1, r, g, b);
	}
}

char ***createCanvas(int w, int h) {
	int i, j, k;
	char ***canvas;
	canvas = (char ***)malloc(w * sizeof(char **));
	for (i = 0; i < w; i++) {
		canvas[i] = (char **)malloc(h * sizeof(char *));
		for (j = 0; j < h; j++) {
			canvas[i][j] = (char *)malloc(3 * sizeof(char));
			for (k = 0; k < 3; k++)
				canvas[i][j][k] = 255;
		}
	}
	return canvas;
}

void canvasToPPM(char ***canvas, int w, int h, char *filename) {
	FILE *out;
	int i, j;
	out = fopen(filename, "w");
	fprintf(out, "P6\n%d %d\n255\n", w, h);
	for (j = h - 1; j >= 0; j--)
		for (i = 0; i < w; i++)
			fwrite(canvas[i][j], 1, 3, out);
	fclose(out);
}

int _tmain(int argc, _TCHAR* argv[])
{
	int w, h;
	w = 1000;
	h = 1000;
	float d = 0.2; // Distancia entre ojos

	vector<int> lineEndings;
	Matrix *mx = matrixFromRaw("cube.raw", &lineEndings);
	Matrix *mx2 = LinearTransform::getViewportTransform(w, h);
	Matrix *mx3 = mx2->multiply(LinearTransform::getOrthographicTransform(-5, 5, -5, 5, -5, 5));
	Matrix *mx4 = mx3->multiply(LinearTransform::getPerspectiveTransform(-2, 2));

	Point eye;
	eye.x = 2;
	eye.y = 0;
	eye.z = 0.1;

	Point eye2;
	eye2.x = 2;
	eye2.y = d; // Distancia entre ojos
	eye2.z = 0.1;

	Point gaze;
	gaze.x = -1;
	gaze.y = 0;
	gaze.z = -0.3;

	Point gaze2;
	gaze2.x = -1;
	gaze2.y = 0;
	gaze2.z = -0.3;

	Point top;
	top.x = 0;
	top.y = 0;
	top.z = 1;

	Matrix *mx5 = mx4->multiply(LinearTransform::getCameraTransform(eye, gaze, top));
	Matrix *mx52 = mx4->multiply(LinearTransform::getCameraTransform(eye2, gaze2, top));

	char ***canvas = createCanvas(w, h);

	Matrix *prev = NULL;

	for (int i = 0, k = 0; i < mx->cols; i++) {
		Matrix *point = new Matrix(4, 1);
		point->setValue(0, 0, mx->getValue(0, i));
		point->setValue(1, 0, mx->getValue(1, i));
		point->setValue(2, 0, mx->getValue(2, i));
		point->setValue(3, 0, 1);
		Matrix *r = mx5->multiply(point);

		if (prev != NULL) {
			drawLine(canvas, prev->getValue(0, 0) / prev->getValue(3, 0), prev->getValue(1, 0) / prev->getValue(3, 0), r->getValue(0, 0) / r->getValue(3, 0), r->getValue(1, 0) / r->getValue(3, 0), 255, 0, 0);
		}

		if (k < lineEndings.size() && i == lineEndings[k]) {
			prev = NULL;
			k++;
		} else {
			prev = r;
		}
	}

	prev = NULL;

	for (int i = 0, k = 0; i < mx->cols; i++) {
		Matrix *point = new Matrix(4, 1);
		point->setValue(0, 0, mx->getValue(0, i));
		point->setValue(1, 0, mx->getValue(1, i));
		point->setValue(2, 0, mx->getValue(2, i));
		point->setValue(3, 0, 1);
		Matrix *r = mx52->multiply(point);

		if (prev != NULL) {
			drawLine(canvas, prev->getValue(0, 0) / prev->getValue(3, 0), prev->getValue(1, 0) / prev->getValue(3, 0), r->getValue(0, 0) / r->getValue(3, 0), r->getValue(1, 0) / r->getValue(3, 0), 0, 255, 255);
		}

		if (k < lineEndings.size() && i == lineEndings[k]) {
			prev = NULL;
			k++;
		} else {
			prev = r;
		}
	}

	canvasToPPM(canvas, w, h, "res.ppm");
}

