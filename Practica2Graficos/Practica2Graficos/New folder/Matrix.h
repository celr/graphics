#pragma once
#include <vector>
using namespace std;

// Represents a dynamically sized matrix.
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