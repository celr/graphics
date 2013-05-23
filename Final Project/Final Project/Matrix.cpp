#include "stdafx.h"
#include "Matrix.h"

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
