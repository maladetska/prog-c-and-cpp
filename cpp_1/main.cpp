#include "phonebook.h"
#include "quicksort.h"
#include "return_codes.h"

#include <fstream>
#include <vector>

template< typename T >
void scan_vector(std::ifstream& fin, std::vector< T >& vector, const size_t size)
{
	for (int i = 0; i < size; i++)
	{
		fin >> vector[i];
	}
	fin.close();
}

template< typename T >
void sort_vector(std::vector< T >& vector, const std::string& sort_mode, const size_t size)
{
	if (sort_mode == "descending")
	{
		quicksort< T, true >(vector, 0, size - 1);
	}
	else
	{
		quicksort< T, false >(vector, 0, size - 1);
	}
}

template< typename T >
void print_vector(std::ofstream& fout, std::vector< T >& vector, const size_t size)
{
	for (int i = 0; i < size; i++)
	{
		fout << vector[i] << "\n";
	}
	fout.close();
}

template< typename T >
int vector_work(std::ifstream& fin, const std::string& argv2, const std::string& sort_mode, const size_t size)
{
	std::vector< T > vector{};
	try
	{
		vector.resize(size);	// ==> (vector.capacity() == size)
	} catch (const std::bad_alloc& e)
	{
		fin.close();
		return ERROR_MEMORY;
	} catch (const std::length_error& e)
	{
		fin.close();
		return ERROR_MEMORY;
	}

	scan_vector(fin, vector, size);

	sort_vector(vector, sort_mode, size);

	std::ofstream fout(argv2);
	if (!fout.is_open())
	{
		fin.close();
		return ERROR_FILE_NOT_FOUND;
	}
	print_vector(fout, vector, size);

	return ERROR_SUCCESS;
}

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		return ERROR_INVALID_PARAMETER;
	}

	std::ifstream fin(argv[1]);
	if (!fin.is_open())
	{
		return ERROR_FILE_NOT_FOUND;
	}

	std::string type;
	std::string sort_mode;
	size_t size;

	fin >> type;
	fin >> sort_mode;
	fin >> size;

	int res;
	switch (type[0])
	{
	case 'i':
		res = vector_work< int >(fin, argv[2], sort_mode, size);
		return res;
	case 'f':
		res = vector_work< float >(fin, argv[2], sort_mode, size);
		return res;
	case 'p':
		res = vector_work< Phonebook >(fin, argv[2], sort_mode, size);
		return res;
	default:
		fin.close();
		return ERROR_INVALID_DATA;
	}
}