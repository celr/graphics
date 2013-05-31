// Curves.cpp : Program for drawing Bézier and Hermit curves.
//
// Carlos López Rivas
// Adonais Romero González
//
// This program reads a in.curve file representing a curve and generates a PPM picture from it.
// The program draws a Bézier curve by default, the user can pass the -hc command line argument to draw a Hermit curve instead.
// The program can receive many parameters from command line:
//     -w [width]
//         Width of the image result.
//     -h [height]
//         Height of the image result.
//     -n [lines]
//         Number of lines to draw (100 by default) 
//     -hc
//         Draw a Hermit curve instead of a Bézier curve.

#include <fstream>
#include <vector>

using namespace std;

// Matrices
class Matrix
{
private:
	vector<float> *v;
public:
	int rows, cols;
	float getValue(int row, int col);
	void setValue(int row, int col, float val);
	Matrix *multiply(Matrix *m);
	Matrix *cross(Matrix *m);
	void Matrix::addColumn();
	Matrix(int rows, int cols);
	~Matrix(void);
};

Matrix *Matrix::multiply(Matrix *m) {
	Matrix *res = new Matrix(this->rows, m->cols);
	for (int i = 0; i < res->rows; i++) {
		for (int k = 0; k < res->cols; k++) {
			res->setValue(i, k, 0);
			for (int p = 0; p < this->cols; p++) {
				float currValue = res->getValue(i, k);
				res->setValue(i, k, currValue + this->getValue(i, p) * m->getValue(p, k));
			}
		}
	}

	return res;
}

float Matrix::getValue(int row, int col) {
	return (*this->v)[ col * this->rows+ row];
}

void Matrix::setValue(int row, int col, float val) {
	(*this->v)[ col * this->rows + row] = val;
}

void Matrix::addColumn() {
	for (int i = 0; i < this->rows; i++) {
		this->v->push_back(0);
	}
	this->cols++;
}

Matrix::Matrix(int rows, int cols)
{
	this->cols = cols;
	this->rows = rows;
	this->v = new vector<float>();
	for (int i = 0; i < this->rows; i++) {
		for (int k = 0; k < this->cols; k++) {
			this->v->push_back(0.0);
		}
	}
}


Matrix::~Matrix(void)
{
	delete this->v;
}

// Canvas manipulation functions
typedef struct
{
	unsigned short r;
	unsigned short g;
	unsigned short b;
	double i;
	unsigned short k;
} Color;

typedef struct
{
	Color **col;
	int w;
	int h;
	double **z;
	bool zEnabled;
} Canvas;

const Color COLOR_BLACK = {0, 0, 0};
const Color COLOR_WHITE = {255, 255, 255};

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
	c->zEnabled;
	return c;
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

class Point {
public:
	float x;
	float y;
};

class Curve {
public:
	Point p1; // Initial point
	Point p2; // Final point
	Point c1; // Control point 1
	Point c2; // Control point 2
};

Curve curveFromFile(char *filename) {
	Curve c;
	ifstream file(filename);
	file >> c.p1.x >> c.p1.y >> c.p2.x >> c.p2.y >> c.c1.x >> c.c1.y >> c.c2.x >> c.c2.y;
	return c;
}

enum curveType {
	Bezier,
	Hermit
};

void drawCurve(Canvas *canvas, Curve curve, int n, curveType type) {
	Matrix *g = new Matrix(4, 2);
	Matrix *m = new Matrix(4, 4);

	if (type == Bezier) {
		// g = p1
		//     c1
		//     c2
		//     p2
		g->setValue(0, 0, curve.p1.x);
		g->setValue(0, 1, curve.p1.y);
		g->setValue(1, 0, curve.c1.x);
		g->setValue(1, 1, curve.c1.y);
		g->setValue(2, 0, curve.c2.x);
		g->setValue(2, 1, curve.c2.y);
		g->setValue(3, 0, curve.p2.x);
		g->setValue(3, 1, curve.p2.y);

		// Bézier constant matrix
		// m = -1  3 -3  1
		//      3 -6  3  0
		//     -3  3  0  0
		//      1  0  0  0
		m->setValue(0, 0, -1);
		m->setValue(0, 1, 3);
		m->setValue(0, 2, -3);
		m->setValue(0, 3, 1);
		m->setValue(1, 0, 3);
		m->setValue(1, 1, -6);
		m->setValue(1, 2, 3);
		m->setValue(2, 0, -3);
		m->setValue(2, 1, 3);
		m->setValue(3, 0, 1);
	} else if (type == Hermit) {
		// g = p1
		//     p2
		//     c1
		//     c2
		g->setValue(0, 0, curve.p1.x);
		g->setValue(0, 1, curve.p1.y);
		g->setValue(1, 0, curve.p2.x);
		g->setValue(1, 1, curve.p2.y);
		g->setValue(2, 0, curve.c1.x);
		g->setValue(2, 1, curve.c1.y);
		g->setValue(3, 0, curve.c2.x);
		g->setValue(3, 1, curve.c2.y);

		// Hermit constant matrix
		// m =  2 -2  1  1
		//     -3  3 -2 -1
		//      0  0  1  0
		//      1  0  0  0
		m->setValue(0, 0, 2);
		m->setValue(0, 1, -2);
		m->setValue(0, 2, 1);
		m->setValue(0, 3, 1);
		m->setValue(1, 0, -3);
		m->setValue(1, 1, 3);
		m->setValue(1, 2, -2);
		m->setValue(1, 3, -1);
		m->setValue(2, 2, 1);
		m->setValue(3, 0, 1);
	}

	Matrix *mg = m->multiply(g);
	float increment = 1/float(n);
	Point *prev = NULL;

	for (int i = 0; i < n; i++) {
		// t = i^3 i^2 i 1
		float it = increment * (i + 1);
		Matrix *t = new Matrix(1, 4);
		t->setValue(0, 0, pow(it, 3));
		t->setValue(0, 1, pow(it, 2));
		t->setValue(0, 2, it);
		t->setValue(0, 3, 1);

		Matrix *result = t->multiply(mg);

		if (prev != NULL) {
			drawLine(canvas, prev->x, prev->y, result->getValue(0, 0), result->getValue(0, 1), COLOR_WHITE);
			delete prev;
		}

		prev = new Point();
		prev->x = result->getValue(0, 0);
		prev->y = result->getValue(0, 1);
	}
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

int main(int argc, char* argv[])
{
	int w, h, i;
	char filename[1024];
	char rfilename[1024];
	w = 1920;
	h = 1080;
	strcpy(filename, "in");
	curveType type = Bezier;
	int n = 100;
	for (i = 1; i < argc; i++)
	{
		if ((strcmp(argv[i], "-w") == 0) || (strcmp(argv[i], "--width") == 0))
		{
			i++;
			sscanf(argv[i], "%d", &w);
		}
		else if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--height") == 0))
		{
			i++;
			sscanf(argv[i], "%d", &h);
		}
		else if (strcmp(argv[i], "-hc") == 0)
		{
			i++;
			type = Hermit;
		}
		else if ((strcmp(argv[i], "-n") == 0))
		{
			i++;
			sscanf(argv[i], "%d", &n);
		}
		else
		{
			sscanf(argv[i], "%s", filename);
		}
	}

	Curve curve = curveFromFile("in.curve");
	Canvas *canvas = createCanvas(w, h);
	drawCurve(canvas, curve, n, type);
	canvasToPPM(canvas, "out.ppm");
	return 0;
}