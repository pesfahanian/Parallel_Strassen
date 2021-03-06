//Parsa Esfahanian//
//.....................................................................................//

#include "stdafx.h"
#include<iostream>
#include<conio.h>
#include <chrono>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <omp.h>

using namespace std;
using namespace std::chrono;

double **allocate_matrix(int size, int random);
void strassen(double **a, double **b, double **c, int size);
void sum(double **a, double **b, double **result, int size);
void subtract(double **a, double **b, double **result, int size);
double **free_matrix(double **v, int size);

int Max_size = 256;
int chunk = 10;
int nthread = 4;

//.....................................................................................//

int main()
{
	
	srand(time(NULL));
	int size = 2048;
	int tid;
	double **A, **B, **C;

	A = allocate_matrix(size, 1);
	B = allocate_matrix(size, 1);
	C = allocate_matrix(size, -1);

	milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	long long time = ms.count();
	cout << "Started at: " << time << endl;
	omp_set_num_threads(nthread);
	#pragma omp parallel shared(A,B,C)
	{
		tid = omp_get_thread_num();
		if (tid == 0)
		{
			nthread = omp_get_num_threads();
			printf("Processing with %d threads\n", nthread);
		}
		strassen(A, B, C, size);
	}
	ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	cout << "Finished at: " << ms.count() << endl;
	cout << "Duration: " << ms.count() - time << " miliseconds" << endl << endl;
			
	A = free_matrix(A, size);
	B = free_matrix(B, size);
	C = free_matrix(C, size);

	_getch();
	return 0;
}

//.....................................................................................//

double **allocate_matrix(int size, int random)
{

	int i, j, n = size, m = size;
	double **v, a;

	v = (double**)malloc(n * sizeof(double*));

	if (v == NULL) {
		printf("** Error in matrix allocation: insufficient memory **");
		return (NULL);
	}

	for (i = 0; i < n; i++) {
		v[i] = (double*)malloc(m * sizeof(double));

		if (v[i] == NULL) {
			printf("** Error: Insufficient memory **");
			free_matrix(v, n);
			return (NULL);
		}
	}

	if (random > -1)
	{
		#pragma omp for schedule (static, chunk)
		for (i = 0; i < n; i++) {
			for (j = 0; j < m; j++)
				v[i][j] = (rand() % 10)*random;
		}
	}

	return (v);
}

//.....................................................................................//

void normal_multipliction(double **a, double **b, double **c, int n) 
{
	int i, j, k;
	
  #pragma omp for schedule (static, chunk) 
	for (i = 0; i < n; i++) 
	{
		for (k = 0; k < n; k++) 
		{
			for (j = 0; j < n; j++) 
			{
				c[i][j] += a[i][k] * b[k][j];
			}
		}
	}
}

//.....................................................................................//

void strassen(double **a, double **b, double **c, int size)
{

	if (size <= Max_size) {
		normal_multipliction(a, b, c, size);
		return;
	}

	else {
		int i, j, k;
		int New_size = size / 2;
		double **a11, **a12, **a21, **a22;
		double **b11, **b12, **b21, **b22;
		double **c11, **c12, **c21, **c22;
		double **p1, **p2, **p3, **p4, **p5, **p6, **p7;

		double **aResult = allocate_matrix(New_size, -1);
		double **bResult = allocate_matrix(New_size, -1);

		a11 = allocate_matrix(New_size, -1);
		a12 = allocate_matrix(New_size, -1);
		a21 = allocate_matrix(New_size, -1);
		a22 = allocate_matrix(New_size, -1);

		b11 = allocate_matrix(New_size, -1);
		b12 = allocate_matrix(New_size, -1);
		b21 = allocate_matrix(New_size, -1);
		b22 = allocate_matrix(New_size, -1);

		c11 = allocate_matrix(New_size, -1);
		c12 = allocate_matrix(New_size, -1);
		c21 = allocate_matrix(New_size, -1);
		c22 = allocate_matrix(New_size, -1);

		p1 = allocate_matrix(New_size, -1);
		p2 = allocate_matrix(New_size, -1);
		p3 = allocate_matrix(New_size, -1);
		p4 = allocate_matrix(New_size, -1);
		p5 = allocate_matrix(New_size, -1);
		p6 = allocate_matrix(New_size, -1);
		p7 = allocate_matrix(New_size, -1);

  #pragma omp for schedule (static, chunk)
		for (i = 0; i < New_size; i++) {
			for (j = 0; j < New_size; j++) {
				k = i + New_size;
				a11[i][j] = a[i][j];
				a12[i][j] = a[i][j + New_size];
				a21[i][j] = a[k][j];
				a22[i][j] = a[k][j + New_size];

				b11[i][j] = b[i][j];
				b12[i][j] = b[i][j + New_size];
				b21[i][j] = b[k][j];
				b22[i][j] = b[k][j + New_size];
			}
		}

		sum(a11, a22, aResult, New_size);
		sum(b11, b22, bResult, New_size);
		strassen(aResult, bResult, p1, New_size);

		sum(a21, a22, aResult, New_size);
		strassen(aResult, b11, p2, New_size);

		subtract(b12, b22, bResult, New_size);
		strassen(a11, bResult, p3, New_size);

		subtract(b21, b11, bResult, New_size);
		strassen(a22, bResult, p4, New_size);

		sum(a11, a12, aResult, New_size);
		strassen(aResult, b22, p5, New_size);

		subtract(a21, a11, aResult, New_size);
		sum(b11, b12, bResult, New_size);
		strassen(aResult, bResult, p6, New_size);

		subtract(a12, a22, aResult, New_size);
		sum(b21, b22, bResult, New_size);
		strassen(aResult, bResult, p7, New_size);

		sum(p3, p5, c12, New_size);
		sum(p2, p4, c21, New_size);

		sum(p1, p4, aResult, New_size);
		sum(aResult, p7, bResult, New_size);
		subtract(bResult, p5, c11, New_size);

		sum(p1, p3, aResult, New_size);
		sum(aResult, p6, bResult, New_size);
		subtract(bResult, p2, c22, New_size);

		#pragma omp for schedule (static, chunk)
		for (i = 0; i < New_size; i++) {
			for (j = 0; j < New_size; j++) {
				c[i][j] = c11[i][j];
				c[i][j + New_size] = c12[i][j];
				c[i + New_size][j] = c21[i][j];
				c[i + New_size][j + New_size] = c22[i][j];
			}
		}

		a11 = free_matrix(a11, New_size);
		a12 = free_matrix(a12, New_size);
		a21 = free_matrix(a21, New_size);
		a22 = free_matrix(a22, New_size);

		b11 = free_matrix(b11, New_size);
		b12 = free_matrix(b12, New_size);
		b21 = free_matrix(b21, New_size);
		b22 = free_matrix(b22, New_size);

		c11 = free_matrix(c11, New_size);
		c12 = free_matrix(c12, New_size);
		c21 = free_matrix(c21, New_size);
		c22 = free_matrix(c22, New_size);

		p1 = free_matrix(p1, New_size);
		p2 = free_matrix(p2, New_size);
		p3 = free_matrix(p3, New_size);
		p4 = free_matrix(p4, New_size);
		p5 = free_matrix(p5, New_size);
		p6 = free_matrix(p6, New_size);
		p7 = free_matrix(p7, New_size);
		aResult = free_matrix(aResult, New_size);
		bResult = free_matrix(bResult, New_size);
	}
}

//.....................................................................................//

void sum(double **a, double **b, double **result, int size) 
{
	int i, j;

	#pragma omp for schedule (static, chunk)
	for (i = 0; i < size; i++) 
	{
		for (j = 0; j < size; j++) 
		{
			result[i][j] = a[i][j] + b[i][j];
		}
	}
}

//.....................................................................................//

void subtract(double **a, double **b, double **result, int size) 
{
	int i, j;

	#pragma omp for schedule (static, chunk)
	for (i = 0; i < size; i++) 
	{
		for (j = 0; j < size; j++) 
		{
			result[i][j] = a[i][j] - b[i][j];
		}
	}
}

//.....................................................................................//

double **free_matrix(double **v, int size) 
{
	int i;
	if (v == NULL) 
	{
		return (NULL);
	}

	for (i = 0; i < size; i++) 
	{
		if (v[i]) 
		{
			free(v[i]);
			v[i] = NULL;
		}
	}

	free(v);
	v = NULL;

	return (NULL);
}

//.....................................................................................//
//Parsa Esfahanian//
