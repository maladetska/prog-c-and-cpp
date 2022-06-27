#include "return_codes.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(ZLIB)
	#include <zlib.h>
#elif defined(LIBDEFLATE)
	#include <libdeflate.h>
#elif defined(ISAL)
	#error "isa-l not supported"
#endif

int number_of_colors;
uint32_t row;
size_t column;
int number_of_chunks;

typedef struct chunk
{
	uint32_t length;
	char type[4];
	unsigned char *data;
} chunk_t;

typedef struct IHDR
{
	uint32_t width, height;
	uint8_t bit_depth, color_type;
	uint8_t compression_method, filter_method, interlace_method;
} IHDR_t;

typedef struct IDAT
{
	int amount;
	int capacity;
	chunk_t **chunks;
} IDAT_chunks_t;

uint32_t swap_bytes(uint32_t b)
{
	return ((b & 0x000000ff) << 24) | ((b & 0xff000000) >> 24) | ((b & 0x0000ff00) << 8) | ((b & 0x00ff0000) >> 8);
}

int create_number(FILE *output, uint32_t prev_bit)
{
	if (prev_bit)
	{
		int value = 0;
		uint32_t copy_num = prev_bit;
		while (copy_num > 0)
		{
			copy_num /= 10;
			value++;
		}
		uint32_t *new_bits;
		new_bits = (uint32_t *)malloc(sizeof(uint32_t) * value);
		if (new_bits == NULL)
		{
			return ERROR_MEMORY;
		}
		int ind = 0;
		for (int i = 0; i < value; i++)
		{
			new_bits[ind++] = prev_bit % 10;
			prev_bit /= 10;
		}
		for (int i = 0; i < value; i++)
		{
			unsigned char ptr = '0' + new_bits[value - 1 - i];
			fwrite(&ptr, sizeof(char), 1, output);
		}
	}
	else
	{
		fwrite("0", sizeof(char), 1, output);
	}

	return 0;
}

int create_PNM_header(FILE *output, uint32_t width, uint32_t height)
{
	if (create_number(output, width) != 0)
	{
		return ERROR_MEMORY;
	}
	fprintf(output, " ");
	if (create_number(output, height) != 0)
	{
		return ERROR_MEMORY;
	}
	fprintf(output, "\n");
	fprintf(output, "255\n");

	return 0;
}

void free_chunks(chunk_t **chunks)
{
	int ind = 0;
	while (ind < number_of_chunks)
	{
		free(chunks[ind]->data);
		free(chunks[ind++]);
	}
	free(chunks);
}

int chunk_analysis(FILE *input, chunk_t *chunk)
{
	fread(chunk, 4, 2, input);
	chunk->length = swap_bytes(chunk->length);
	chunk->data = malloc(chunk->length);
	if (chunk->data == NULL)
	{
		free(chunk->data);
		return ERROR_MEMORY;
	}
	if (chunk->length == 0)
	{
		chunk->data = NULL;
	}
	else
	{
		fread(chunk->data, 1, chunk->length, input);
	}
	unsigned char other;
	fread(&other, 4, 1, input);

	return 0;
}

int check_PNG_format(FILE *input)
{
	char buffer[8];
	if (fread(&buffer[0], sizeof(char), 8, input) != 8)
	{
		fclose(input);
		return ERROR_INVALID_DATA;
	}
	if (strncmp(&buffer[0], "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a", 8) != 0)
	{
		fclose(input);
		return ERROR_INVALID_DATA;
	}
	return 0;
}

int init_IHDR(FILE *output, IHDR_t *IHDR)
{
	IHDR->width = swap_bytes(IHDR->width);
	IHDR->height = swap_bytes(IHDR->height);
	if (IHDR->compression_method != 0 || IHDR->filter_method != 0 || (IHDR->interlace_method != 0 && IHDR->interlace_method != 1))
	{
		return ERROR_INVALID_DATA;
	}

	switch (IHDR->color_type)
	{
	case 0:
		if (IHDR->bit_depth == 1 || IHDR->bit_depth == 2 || IHDR->bit_depth == 4 || IHDR->bit_depth == 8 || IHDR->bit_depth == 16)
		{
			fprintf(output, "P5\n");
			break;
		}
		else
		{
			return ERROR_INVALID_DATA;
		}
	case 2:
		if (IHDR->bit_depth == 8 || IHDR->bit_depth == 16)
		{
			fprintf(output, "P6\n");
			break;
		}
		else
		{
			return ERROR_INVALID_DATA;
		}
	default:
		return ERROR_INVALID_DATA;
	}
	number_of_colors = IHDR->color_type + 1;
	row = IHDR->height;
	column = IHDR->width * number_of_colors;

	return 0;
}

int work_with_IHDR(FILE *input, FILE *output)
{
	chunk_t IHDR_chunk;
	if (chunk_analysis(input, &IHDR_chunk))
	{
		fclose(input);
		fclose(output);
		return ERROR_MEMORY;
	}
	else if (strncmp(IHDR_chunk.type, "IHDR", 4) != 0)
	{
		fclose(input);
		fclose(output);
		return ERROR_INVALID_DATA;
	}

	IHDR_t *IHDR = (IHDR_t *)IHDR_chunk.data;
	int check_IHDR = init_IHDR(output, IHDR);
	if (check_IHDR != 0)
	{
		free(IHDR);
		fclose(input);
		fclose(output);
		return ERROR_INVALID_DATA;
	}
	if (create_PNM_header(output, IHDR->width, IHDR->height) != 0)
	{
		free(IHDR);
		fclose(input);
		fclose(output);
		return ERROR_MEMORY;
	}
	free(IHDR);

	return 0;
}

int read_IDAT_chunks(FILE *input, FILE *output, IDAT_chunks_t IDAT_chunks, chunk_t **chunks)
{
	IDAT_chunks.amount = 0;
	IDAT_chunks.capacity = 1;
	if (chunks == NULL)
	{
		fclose(input);
		fclose(output);
		return ERROR_MEMORY;
	}

	int iter = 1;
	chunk_t *curr = NULL;
	while (iter == 1 || strncmp(curr->type, "IEND", 4) != 0)
	{
		int check;
		curr = malloc(24);
		if (curr == NULL)
		{
			check = ERROR_MEMORY;
		}
		else
		{
			check = chunk_analysis(input, curr);
		}
		if (check != 0)
		{
			if (curr != NULL)
			{
				free(curr->data);
			}
			free(curr);
			int ind = 0;
			while (ind < IDAT_chunks.amount)
			{
				free(chunks[ind]->data);
				free(chunks[ind++]);
			}
			free(chunks);
			fclose(input);
			fclose(output);
			return ERROR_MEMORY;
		}
		if (strncmp(curr->type, "IDAT", 4) == 0)
		{
			chunks[IDAT_chunks.amount++] = curr;
		}
		if (IDAT_chunks.capacity < IDAT_chunks.amount)
		{
			IDAT_chunks.capacity *= 2;
			chunks = realloc(chunks, IDAT_chunks.capacity * 24);
			if (chunks == NULL)
			{
				free(chunks);
				fclose(input);
				fclose(output);
				return ERROR_MEMORY;
			}
		}
		iter = 0;
	}

	number_of_chunks = IDAT_chunks.amount;

	return 0;
}

int pixel_matrix(FILE *input, FILE *output, unsigned char *data_of_chunk, size_t scale)
{
	unsigned char(*png_data)[column + 1] = (unsigned char *)data_of_chunk;
	int(*pix_matrix)[column] = calloc(scale, sizeof(int));
	/*int **pix_matrix;
	pix_matrix = (int **)malloc(sizeof(int) * scale);*/
	if (pix_matrix == NULL)
	{
		free(png_data);
		free(pix_matrix);
		fclose(input);
		fclose(output);
		return ERROR_MEMORY;
	}
	for (uint32_t y = 0; y < row; y++)
	{
		for (size_t x = 0; x < column; x++)
		{
			pix_matrix[y][x] = (int)png_data[y][x + 1];
			switch (png_data[y][0])
			{
			case 0:
				break;
			case 1:
				if (x >= number_of_colors)
				{
					pix_matrix[y][x] += pix_matrix[y][x - number_of_colors];
				}
				break;
			case 2:
				if (y > 0)
				{
					pix_matrix[y][x] += pix_matrix[y - 1][x];
				}
				break;
			case 3:
				if (x >= number_of_colors)
				{
					pix_matrix[y][x] += pix_matrix[y][x - number_of_colors] / 2;
				}
				if (y > 0)
				{
					pix_matrix[y][x] += pix_matrix[y - 1][x] / 2;
				}
				break;
			case 4:
			{
				int a = 0, b = 0, c = 0;
				if (x >= number_of_colors)
				{
					a = pix_matrix[y][x - number_of_colors];
					if (y > 0)
					{
						c = (pix_matrix[y - 1][x - number_of_colors]);
					}
				}
				if (y > 0)
				{
					b = pix_matrix[y - 1][x];
				}

				int pa = abs(b - c);
				int pb = abs(a - c);
				int pc = abs(a + b - 2 * c);

				if (pa <= pb && pa <= pc)
				{
					pix_matrix[y][x] += a;
				}
				else if (pb <= pc)
				{
					pix_matrix[y][x] += b;
				}
				else
				{
					pix_matrix[y][x] += c;
				}
				break;
			}
			default:
				free(png_data);
				free(pix_matrix);
				fclose(input);
				fclose(output);
				return ERROR_UNKNOWN;
			}
			pix_matrix[y][x] = 0x000000ff & pix_matrix[y][x];
		}
	}

	int x, y = 0;
	while (y < row)
	{
		x = 0;
		while (x < column)
		{
			fwrite(&pix_matrix[y][x++], sizeof(char), 1, output);
		}
		y++;
	}
	free(png_data);
	free(pix_matrix);

	return 0;
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		return ERROR_INVALID_PARAMETER;
	}
	FILE *input = fopen(argv[1], "rb");
	if (!input)
	{
		return ERROR_FILE_NOT_FOUND;
	}

	int check_PNG = check_PNG_format(input);
	if (check_PNG != 0)
	{
		return ERROR_INVALID_DATA;
	}

	FILE *output = fopen(argv[2], "wb");
	if (!output)
	{
		fclose(input);
		return ERROR_FILE_NOT_FOUND;
	}

	int check_IHDR = work_with_IHDR(input, output);
	if (check_IHDR != 0)
	{
		return check_IHDR;
	}

	IDAT_chunks_t IDAT_chunks;
	IDAT_chunks.chunks = malloc(sizeof(chunk_t *) * IDAT_chunks.capacity);
	int check_IDAT = read_IDAT_chunks(input, output, IDAT_chunks, IDAT_chunks.chunks);
	if (check_IDAT != 0)
	{
		return check_IDAT;
	}

	for (int i = 0; i < number_of_chunks; i++)
	{
		if (strncmp(IDAT_chunks.chunks[i]->type, "IDAT", 4) == 0)
		{
			size_t scale = row * (column + 1);
			unsigned char *data_of_chunk = (unsigned char *)malloc(scale);
			if (!data_of_chunk)
			{
				free_chunks(IDAT_chunks.chunks);
				fclose(input);
				fclose(output);
				return ERROR_MEMORY;
			}

#if defined(ZLIB)
			if (uncompress(data_of_chunk, &scale, IDAT_chunks.chunks[i]->data, IDAT_chunks.chunks[i]->length))
			{
				free(data_of_chunk);
				free_chunks(IDAT_chunks.chunks);
				fclose(input);
				fclose(output);
				return ERROR_MEMORY;
			}
#elif defined(LIBDEFLATE)
			struct libdeflate_decompressor *d;
			d = libdeflate_alloc_decompressor();
			if (libdeflate_zlib_decompress(d, IDAT_chunks.chunks[i]->data, IDAT_chunks.chunks[i]->length, data_of_chunk, scale, NULL))
			{
				free(data_of_chunk);
				free_chunks(IDAT_chunks.chunks);
				fclose(input);
				fclose(output);
				return ERROR_MEMORY;
			}
			libdeflate_free_decompressor(d);
#endif
			int check_pixel_matrix = pixel_matrix(input, output, data_of_chunk, scale);
			if (check_pixel_matrix != 0)
			{
				free(data_of_chunk);
				free_chunks(IDAT_chunks.chunks);
				return check_pixel_matrix;
			}
		}
	}
	free_chunks(IDAT_chunks.chunks);
	fclose(input);
	fclose(output);

	return 0;
}