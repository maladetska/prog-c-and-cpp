#pragma once

#include <random>
#include <vector>

template< typename T >
bool cmp_less(const T &item1, const T &item2)
{
	return item1 < item2;
}

template< typename T, bool descending >
void quicksort(std::vector< T > &A, size_t left, size_t right)
{
	// srand(time(NULL));
	const T pivot = A[std::rand() % (right - left + 1) + left];
	size_t i = left;
	size_t j = right;
	while (i < j)
	{
		if (descending)
		{
			while (cmp_less(pivot, A[i]))
			{
				i++;
			}
			while (cmp_less(A[j], pivot))
			{
				j--;
			}
		}
		else
		{
			while (cmp_less(A[i], pivot))
			{
				i++;
			}
			while (cmp_less(pivot, A[j]))
			{
				j--;
			}
		}
		if (i <= j)
		{
			std::swap(A[i++], A[j--]);
		}
	}
	if (left >= j)
	{
		if (right > i)
		{
			quicksort< T, descending >(A, i, right);
		}
	}
	else if (right <= i)
	{
		if (left < j)
		{
			quicksort< T, descending >(A, left, j);
		}
	}
	else
	{
		if (j - left < right - i)
		{
			quicksort< T, descending >(A, left, j);
			quicksort< T, descending >(A, i, right);
		}
		else
		{
			quicksort< T, descending >(A, i, right);
			quicksort< T, descending >(A, left, j);
		}
	}
}
