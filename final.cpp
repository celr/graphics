/*
 * Red-Cyan anaglyph 3D picture drawing
 * Adonais Romero González
 *
 *
 * This program reads a .raw file, representing a 3D model, and generates a red-cyan anaglyph 3D picture from it, saved in PPM format.
 *
 * The program can recieve many parameters as arguments from command line:
 *     [filename]
 *         File name of the files to be read (in, by default).
 *     -w [width]
 *         Width of the image result.
 *     -h [height]
 *         Height of the image result.
 *     c [x] [y] [z]
 *         Indicates the (x, y, z) coordinates of the center of the camera.
 *     d [x] [y] [z]
 *         Direction of projection.
 *     u [x] [y] [z]
 *         Up vector. It must be unitary and perpendicular to the direction of projection.
 *     -s [scale_factor]
 *         The image is scaled to the indicated scale factor.
 *
 * The result is saved in a out.ppm file.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <limits>
#include <list>
#include <vector>
#include <fstream>
using namespace std;

#define BUFFER_SIZE 1024

typedef unsigned char uchar;

typedef struct
{
	unsigned short r;
	unsigned short g;
	unsigned short b;
	double i;
	unsigned short k;
} Color;


const Color COLOR_BLACK = {0, 0, 0};
const Color COLOR_WHITE = {255, 255, 255};

typedef struct
{
	double **val;
	int r;
	int c;
} Matrix;

typedef struct
{
	double x;
	double y;
	double z;
} Point3D;

typedef struct
{
	Color **col;
	int w;
	int h;
	double **z;
} Canvas;

typedef list<Point3D> Face3D;
typedef list<Face3D> Map3D;

void swap(int *, int *);

Canvas *createCanvas(int, int);
void canvasToPPM(Canvas *, const char *);
void drawPixel(Canvas *, int, int, Color);
void drawPixelZ(Canvas *, int, int, double, Color);
void drawLine(Canvas *, int, int, int, int, Color);
void drawTriangle(Canvas *, int, int, double, int, int, double, int, int, double, Color);

Matrix *createMatrix(int r, int c)
{
	int i, j;
	Matrix *m;
	m = new Matrix;
	m->val = new double*[r];
	for (i = 0; i < r; i++)
	{
		m->val[i] = new double[c];
		for (j = 0; j < c; j++)
		{
			m->val[i][j] = 0.0;
		}
	}
	m->r = r;
	m->c = c;
	return m;
}

Matrix *multMatrix(Matrix *a, Matrix *b)
{
	int i, j, k, l;
	Matrix *c;
	if (a->c != b->r)
	{
		return NULL;
	}
	l = a->c;
	c = createMatrix(a->r, b->c);
	for (i = 0; i < c->r; i++)
	{
		for (j = 0; j < c->c; j++)
		{
			for (k = 0; k < l; k++)
			{
				c->val[i][j] += a->val[i][k] * b->val[k][j];
			}
		}
	}
	return c;
}

Matrix *point3DToMatrix(Point3D p)
{
	Matrix *m;
	m = createMatrix(4, 1);
	m->val[0][0] = p.x;
	m->val[1][0] = p.y;
	m->val[2][0] = p.z;
	m->val[3][0] = 1.0;
	return m;
}

Map3D *openRawMap(const char *filename)
{
	FILE *fp;
	Point3D *p;
	Face3D *f;
	Map3D *m;
	double x;
	char buffer[BUFFER_SIZE];
	char *c;
	fp = fopen(filename, "r");
	m = new Map3D;
	while (fgets(buffer, BUFFER_SIZE - 1, fp) != NULL)
	{
		f = new Face3D;
		c = buffer;
		while (true)
		{
			x = strtod(c, &c);
			if (*(c + 1) == '\n')
			{
				break;
			}
			p = new Point3D;
			p->x = x;
			p->y = strtod(c, &c);
			p->z = strtod(c, &c);
			f->push_back(*p);
		};
		m->push_back(*f);
	}
	return m;
}

Matrix *createIdentityMatrix(int l)
{
	int i;
	Matrix *m;
	m = createMatrix(l, l);
	for (i = 0; i < l; i++)
	{
		m->val[i][i] = 1.0;
	}
	return m;
}

double dot3D(Point3D a, Point3D b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

Point3D cross3D(Point3D a, Point3D b)
{
	Point3D c;
	c.x = a.y * b.z - a.z * b.y;
	c.y = a.z * b.x - a.x * b.z;
	c.z = a.x * b.y - a.y * b.x;
	return c;
}

Matrix *createProjectionMatrix(Point3D center, Point3D dir, Point3D up, double near, double far)
{
	Matrix *m, *t_1, *t_2;
	Point3D p;
	
	t_1 = createIdentityMatrix(4);
	t_1->val[0][3] = -center.x;
	t_1->val[1][3] = -center.y;
	t_1->val[2][3] = -center.z;
	
	p = cross3D(up, dir);
	
	t_2 = createMatrix(4, 4);
	t_2->val[0][0] = p.x;
	t_2->val[1][0] = up.x;
	t_2->val[2][0] = dir.x;
	t_2->val[0][1] = p.y;
	t_2->val[1][1] = up.y;
	t_2->val[2][1] = dir.y;
	t_2->val[0][2] = p.z;
	t_2->val[1][2] = up.z;
	t_2->val[2][2] = dir.z;
	t_2->val[3][3] = 1.0;
	
	m = multMatrix(t_2, t_1);
	delete t_1;
	delete t_2;
	t_1 = m;
	
	t_2 = createMatrix(4, 4);
	t_2->val[0][0] = near;
	t_2->val[1][1] = near;
	t_2->val[2][2] = near + far;
	t_2->val[2][2] = -far * near;
	t_2->val[3][2] = 1.0;
	
	m = multMatrix(t_2, t_1);
	delete t_1;
	delete t_2;
	
	return m;
}

void drawScaledLine(Canvas *canvas, double x_1, double y_1, double x_2, double y_2, double scale_1, double scale_2, Color c)
{
	drawLine(canvas, canvas->w / 2 + x_1 * scale_1, canvas->h / 2 + y_1 * scale_1, canvas->w / 2 + x_2 * scale_2, canvas->h / 2 + y_2 * scale_2, c);
}

void drawScaledTriangle(Canvas *canvas, double x_1, double y_1, double z_1, double x_2, double y_2, double z_2, double x_3, double y_3, double z_3, double scale_1, double scale_2, double scale_3, Color c)
{
	int mw = canvas->w / 2;
	int mh = canvas->h / 2;
	drawTriangle(canvas, mw + x_1 * scale_1, mh + y_1 * scale_1, z_1, mw + x_2 * scale_2, mh + y_2 * scale_2, z_2, mw + x_3 * scale_3, mh + y_3 * scale_3, z_3, c);
}

Point3D pointDiff(Point3D a, Point3D b)
{
	Point3D c;
	c.x = a.x - b.x;
	c.y = a.y - b.y;
	c.z = a.z - b.z;
	return c;
}

double magnitude(Point3D a)
{
	return sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

void drawFilledMap3D(Canvas *canvas, Map3D *map, Point3D dir, Matrix *t, double scale, vector<Color> material)
{
	int k;
	Map3D::iterator i;
	Face3D::iterator j;
	Face3D f;
	Point3D p_1, p_2, p_3, v_1, v_2;
	Point3D n;
	Matrix *pm, *r_1, *r_2, *r_3;
	double dotP;
	Color c;
	double intensity;
	for (i = map->begin(), k = 0; i != map->end(); i++, k = (k + 1) % material.size())
	{
		f = *i;
		j = f.begin();
		p_1 = *j;
		j++;
		p_2 = *j;
		j++;
		p_3 = *j;
		v_1 = pointDiff(p_1, p_2);
		v_2 = pointDiff(p_1, p_3);
		n = cross3D(v_1, v_2);
		dotP = -dot3D(n, dir);
		if (dotP > 0.0)
		{
			pm = point3DToMatrix(p_1);
			r_1 = multMatrix(t, pm);
			pm = point3DToMatrix(p_2);
			r_2 = multMatrix(t, pm);
			pm = point3DToMatrix(p_3);
			r_3 = multMatrix(t, pm);
			intensity = pow((dotP/(magnitude(n)*magnitude(dir))), material[k].k);
			if (intensity < 0.05)
			{
				intensity = 0.05;
			}
			c = material[k];
			c.r *= intensity * c.i;
			c.g *= intensity * c.i;
			c.b *= intensity * c.i;
			drawScaledTriangle(canvas, r_1->val[0][0], r_1->val[1][0], r_1->val[2][0], r_2->val[0][0], r_2->val[1][0], r_2->val[2][0], r_3->val[0][0], r_3->val[1][0], r_3->val[2][0], scale / r_1->val[3][0], scale / r_2->val[3][0], scale / r_3->val[3][0], c);
		}
	}
}

vector<Color> colorsFromMaterial(char *fileName) {
	vector<Color> colors;
	ifstream file;
	file.open(fileName);
	Color color;

	while(file >> color.r >> color.g >> color.b >> color.i >> color.k) {
		colors.push_back(color);
	}

	file.close();
	
	return colors;
}

void cameraFromFile(Point3D *pos, Point3D *dir, Point3D *up, char *fileName) {
	ifstream file;
	file.open(fileName);
	file >> pos->x >> pos->y >> pos->z;
	file >> dir->x >> dir->y >> dir->z;
	file >> up->x >> up->y >> up->z;
	file.close();
}

void drawMap3D(Canvas *canvas, Map3D *map, Point3D dir, Matrix *t, double scale)
{
	Map3D::iterator i;
	Face3D::iterator j;
	Face3D f;
	Point3D p_1, p_2, p_3, v_1, v_2;
	Point3D n;
	Matrix *pm, *r_1, *r_2, *r_3;
	for (i = map->begin(); i != map->end(); i++)
	{
		f = *i;
		j = f.begin();
		p_1 = *j;
		j++;
		p_2 = *j;
		j++;
		p_3 = *j;
		v_1 = pointDiff(p_1, p_2);
		v_2 = pointDiff(p_1, p_3);
		n = cross3D(v_1, v_2);
		/*if (dot3D(*n, dir) < 0.0)
		{*/
			pm = point3DToMatrix(p_1);
			r_1 = multMatrix(t, pm);
			pm = point3DToMatrix(p_2);
			r_2 = multMatrix(t, pm);
			pm = point3DToMatrix(p_3);
			r_3 = multMatrix(t, pm);
			drawScaledLine(canvas, r_1->val[0][0], r_1->val[1][0], r_2->val[0][0], r_2->val[1][0], scale / r_1->val[3][0], scale / r_2->val[3][0], COLOR_WHITE);
			drawScaledLine(canvas, r_1->val[0][0], r_1->val[1][0], r_3->val[0][0], r_3->val[1][0], scale / r_1->val[3][0], scale / r_3->val[3][0], COLOR_WHITE);
			drawScaledLine(canvas, r_2->val[0][0], r_2->val[1][0], r_3->val[0][0], r_3->val[1][0], scale / r_2->val[3][0], scale / r_3->val[3][0], COLOR_WHITE);
		/*}*/
	}
}

int main(int argc, char **argv) {
	Map3D *map;
	Point3D center = {0.0, 0.0, 0.0};
	Point3D dir = {0.0, 0.0, 1.0};
	Point3D up = {0.0, 1.0, 0.0};
	Canvas *canvas;
	int w, h;
	int i;
	double scale;
	bool wireframe = false;
	char filename[1024];
	char rfilename[1024];
	w = 1920;
	h = 1080;
	scale = 500.0;
	strcpy(filename, "in");
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
		else if (strcmp(argv[i], "c") == 0)
		{
			i++;
			sscanf(argv[i], "%lf", &center.x);
			i++;
			sscanf(argv[i], "%lf", &center.y);
			i++;
			sscanf(argv[i], "%lf", &center.z);
		}
		else if (strcmp(argv[i], "d") == 0)
		{
			i++;
			sscanf(argv[i], "%lf", &dir.x);
			i++;
			sscanf(argv[i], "%lf", &dir.y);
			i++;
			sscanf(argv[i], "%lf", &dir.z);
		}
		else if (strcmp(argv[i], "u") == 0)
		{
			i++;
			sscanf(argv[i], "%lf", &up.x);
			i++;
			sscanf(argv[i], "%lf", &up.y);
			i++;
			sscanf(argv[i], "%lf", &up.z);
		}
		else if ((strcmp(argv[i], "-s") == 0) || (strcmp(argv[i], "--scale") == 0))
		{
			i++;
			sscanf(argv[i], "%lf", &scale);
		}
		else if ((strcmp(argv[i], "-W") == 0) || (strcmp(argv[i], "--wireframe") == 0))
		{
			wireframe = true;
		}
		else
		{
			sscanf(argv[i], "%s", filename);
		}
	}
	canvas = createCanvas(w, h);
	strcpy(rfilename, filename);
	strcat(rfilename, ".raw");
	map = openRawMap(rfilename);
	strcpy(rfilename, filename);
	strcat(rfilename, ".camera");
	cameraFromFile(&center, &dir, &up, rfilename);
	Matrix *t;
	t = createProjectionMatrix(center, dir, up, -2, 2);
	if (wireframe)
	{
		drawMap3D(canvas, map, dir, t, scale);
	}
	else
	{
		strcpy(rfilename, filename);
		strcat(rfilename, ".material");
		vector<Color> cl = colorsFromMaterial(rfilename);
		drawFilledMap3D(canvas, map, dir, t, scale, cl);
	}
	canvasToPPM(canvas, "out.ppm");
	return EXIT_SUCCESS;
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

