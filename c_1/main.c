#include "return_codes.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int n;
const double eps = 0.00001;

void swap(float *num1, float *num2)
{
	float tmp = *num1;
	*num1 = *num2;
	*num2 = tmp;
}

int scanSLE(FILE *in, float **coefs, float *y)
{
	for (int i = 0; i < n; i++)
	{
		coefs[i] = (float *)malloc(sizeof(int) * n);
		if (coefs[i] == NULL)
		{
			return 1;
		}
		for (int j = 0; j < n; j++)
		{
			fscanf(in, "%f ", &coefs[i][j]);
		}
		fscanf(in, "%f ", &y[i]);
	}
	return 0;
}

void triangularForm(float **coefs, float *y)
{
	for (int i = 0; i < n; i++)
	{
		int idx = i;
		float max = fabsf(coefs[i][i]);
		for (int j = i + 1; j < n; j++)
		{
			if (fabsf(coefs[j][i]) > max)
			{
				idx = i;
				max = fabsf(coefs[j][i]);
			}
		}

		for (int j = 0; j < n; j++)
		{
			swap(&coefs[i][j], &coefs[idx][j]);
		}
		swap(&y[i], &y[idx]);

		for (int j = i; j < n; j++)
		{
			if (fabsf(coefs[j][i]) > eps)
			{
				float num = coefs[j][i];

				for (int k = 0; k < n; k++)
				{
					coefs[j][k] /= num;
				}
				y[j] /= num;

				if (j != i)
				{
					for (int k = 0; k < n; k++)
					{
						coefs[j][k] -= coefs[i][k];
					}
					y[j] -= y[i];
				}
			}
		}
	}
}

int checkSolutions(float **coefs, float *y)
{
	int last = n - 1;
	int flag = 0;

	if (fabsf(coefs[last][last] - coefs[last - 1][last]) < eps)
	{
		coefs[last][last] -= coefs[last - 1][last];
		if (fabsf(y[last] - y[last - 1]) < eps)
		{
			y[last] -= y[last - 1];
		}
	}

	for (int j = 0; j < n; j++)
	{
		if (fabsf(coefs[last][j]) > eps)
		{
			flag++;
		}
	}

	if (flag == 0)
	{
		if (fabsf(y[last]) < eps)
		{
			return 1;
		}
		else
		{
			return 2;
		}
	}

	return 0;
}

void backwardSubstitution(float **coefs, float *y, float *ans)
{
	for (int i = n - 1; i >= 0; i--)
	{
		ans[i] = y[i];
		for (int j = 0; j < i; j++)
			y[j] -= coefs[j][i] * ans[i];
	}
}

int gaussianElimination(float **coefs, float *y, float *ans)
{
	triangularForm(coefs, y);
	int check = checkSolutions(coefs, y);
	if (check != 0)
	{
		return check;
	}
	backwardSubstitution(coefs, y, ans);

	return 0;
}

void printAnswer(FILE *out, float *ans)
{
	for (int i = 0; i < n; i++)
	{
		fprintf(out, "%g\n", ans[i]);
	}
}

void freeMatrix(float **matrix)
{
	for (int i = 0; i < n; i++)
	{
		free(matrix[i]);
	}
	free(matrix);
}

void freeComplex(FILE *in, FILE *out, float **coefs, float *y, float *ans)
{
	freeMatrix(coefs);
	free(y);
	free(ans);
	fclose(in);
	fclose(out);
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		return ERROR_INVALID_PARAMETER;
	}

	FILE *in = fopen(argv[1], "r");
	if (!in)
	{
		return ERROR_FILE_NOT_FOUND;
	}

	FILE *out = fopen(argv[2], "w");
	if (!out)
	{
		fclose(in);
		return ERROR_FILE_NOT_FOUND;
	}

	fscanf(in, "%d", &n);

	float **coefs;
	coefs = (float **)malloc(sizeof(float *) * n * n);
	if (coefs == NULL)
	{
		fclose(in);
		fclose(out);
		return ERROR_MEMORY;
	}

	float *y;
	y = (float *)malloc(sizeof(float) * n);
	if (y == NULL)
	{
		freeMatrix(coefs);
		fclose(in);
		fclose(out);
		return ERROR_MEMORY;
	}

	float *ans;
	ans = (float *)malloc(sizeof(float) * n);
	if (ans == NULL)
	{
		freeMatrix(coefs);
		free(y);
		fclose(in);
		fclose(out);
		return ERROR_MEMORY;
	}

	if (scanSLE(in, coefs, y))
	{
		freeComplex(in, out, coefs, y, ans);
		return ERROR_MEMORY;
	}

	int check = gaussianElimination(coefs, y, ans);
	if (check == 1)
	{
		fprintf(out, "many solution");
		freeComplex(in, out, coefs, y, ans);
		return 0;
	}
	if (check == 2)
	{
		fprintf(out, "no solution");
		freeComplex(in, out, coefs, y, ans);
		return 0;
	}
	printAnswer(out, ans);

	freeComplex(in, out, coefs, y, ans);

	return 0;
}