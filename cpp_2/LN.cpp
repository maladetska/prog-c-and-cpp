#include "LN.h"

LN operator"" _ln(const char *number)
{
	return LN(number);
}

LN::LN(const LN &number) : sign(number.sign), size(number.size), is_nan(number.is_nan)
{
	data = new char[size];
	std::memcpy(data, number.data, size);
}

LN::LN(LN &&number) noexcept
{
	std::swap(sign, number.sign);
	std::swap(data, number.data);
	std::swap(is_nan, number.is_nan);
	std::swap(size, number.size);
}

LN &LN::operator=(const LN &number)
{
	if (this == &number)
	{
		return *this;
	}

	delete_data();
	*this = LN(number);

	return *this;
}

LN &LN::operator=(LN &&number) noexcept
{
	if (this == &number)
	{
		return *this;
	}

	data = number.data;
	number.data = nullptr;
	std::swap(sign, number.sign);

	size = number.size;
	is_nan = number.is_nan;

	return *this;
}

LN::LN(long long number)
{
	size = 1;
	for (long long i = 10; number / i != 0; size++, i *= 10)
		;

	sign = (number >= 0);

	data = new char[size];

	for (size_t i = 0; i < size; i++)
	{
		data[i] = static_cast< char >(number % 10);
		number /= 10;
	}
}

LN::LN(const char *number)
{
	if (number[0] == 'N')
	{
		is_nan = true;
		return;
	}

	size_t zero_count = 0;
	size_t i = 0;
	while (number[i] == '0')
	{
		zero_count++;
		i++;
	}

	if (number[i] == '\0' && number[i - 1] == '0')
	{
		size = 1;
		data = new char[size];
		data[0] = '0';

		return;
	}

	const char *end = number + zero_count;
	while (*end != '\0')
	{
		end++;
	}

	size = 1 > (end - number - zero_count) ? 1 : (end - number - zero_count);

	data = new char[size];

	char *pos;
	pos = new char[size];

	for (const char *cur_char = number + zero_count; *cur_char != '\0'; cur_char++)
	{
		pos[cur_char - number - zero_count] = *cur_char;
	}

	for (i = 0; i < size; i++)
	{
		data[i] = pos[size - 1 - i];
	}

	delete[] pos;
}

[[maybe_unused]] LN::LN(const std::string_view &number) : LN(number.data()) {}

LN::~LN()
{
	delete_data();
}

LN::operator long long() const noexcept(false)
{
	static const constexpr size_t MAX_LONG_LONG_LENGTH = 19;
	static const constexpr long long MAX_LONG_LONG = 9'223'372'036'854'775'807L;
	static const constexpr long long MIN_LONG_LONG = -9'223'372'036'854'775'807L - 1;

	if (size > MAX_LONG_LONG_LENGTH)
	{
		throw std::bad_cast();
	}

	static const LN MAX_LN{ MAX_LONG_LONG };
	static const LN MIN_LN{ MIN_LONG_LONG };

	if (*this < MIN_LN || *this > MAX_LN)
	{
		throw std::bad_cast();
	}

	return 0;
}

LN::operator bool() const
{
	if (size == 0)
	{
		return false;
	}
	else if (size == 1)
	{
		return data[0] != '0';
	}
	return true;
}

LN LN::operator-() const
{
	LN result{ *this };
	if (is_nan)
	{
		return result;
	}
	if (this->size == 1 && this->data[0] == '0')
	{
		result.sign = true;
	}
	else
	{
		result.sign = !this->sign;
	}

	return result;
}

LN LN::operator+(const LN &number) const
{
	if (is_nan)
	{
		return *this;
	}
	if (number.is_nan)
	{
		return number;
	}

	size_t pos_size = (size > number.size) ? size : number.size;

	if (sign == number.sign)
	{
		char *answer = nullptr;
		LN result{ sum(data, number.data, size, number.size, answer, pos_size, (int)(size - number.size)) };
		delete[] answer;

		return !sign ? LN(-result) : result;
	}
	else if (sign && !number.sign)
	{
		if (*this == (-number))
		{
			return 0_ln;
		}
		else if (*this < (-number))
		{
			char *answer = nullptr;
			LN result{ sub(number.data, data, size, answer, pos_size) };
			delete[] answer;

			return LN(-result);
		}
		else
		{
			char *answer = nullptr;
			LN result{ sub(data, number.data, number.size, answer, pos_size) };
			delete[] answer;

			return result;
		}
	}
	else
	{
		if (number == (-*this))
		{
			return 0_ln;
		}
		else if (number < (-*this))
		{
			char *answer = nullptr;
			LN result{ sub(data, number.data, number.size, answer, pos_size) };
			delete[] answer;

			return LN(-result);
		}
		else
		{
			char *answer = nullptr;
			LN result{ sub(number.data, data, size, answer, pos_size) };
			delete[] answer;

			return result;
		}
	}
}

LN LN::operator-(const LN &number) const
{
	if (is_nan)
	{
		return *this;
	}
	if (number.is_nan)
	{
		return number;
	}

	size_t ans_size = (size > number.size) ? size : number.size;

	if (sign != number.sign)
	{
		char *answer = nullptr;
		LN result{ sum(data, number.data, size, number.size, answer, ans_size, (int)(size - number.size)) };
		delete[] answer;

		return sign ? result : LN(-result);
	}
	else if (sign)
	{
		if (*this == number)
		{
			return 0_ln;
		}
		else if (*this < number)
		{
			char *answer = nullptr;
			LN result{ sub(number.data, data, size, answer, ans_size) };
			delete[] answer;

			return LN(-result);
		}
		else
		{
			char *answer = nullptr;
			LN result{ sub(data, number.data, number.size, answer, ans_size) };
			delete[] answer;

			return result;
		}
	}
	else
	{
		if (*this == number)
		{
			return 0_ln;
		}
		else if (*this < number)	// -a < -b,  -a - (-b) = b - a
		{
			char *answer = nullptr;
			LN result{ sub(data, number.data, number.size, answer, ans_size) };
			delete[] answer;

			return LN(-result);
		}
		else	// -a > -b,  -a - (-b) = b - a
		{
			char *answer = nullptr;
			LN result{ sub(number.data, data, size, answer, ans_size) };
			delete[] answer;

			return result;
		}
	}
}

LN LN::operator*(const LN &number) const
{
	if (is_nan || number.is_nan)
	{
		return LN("NaN");
	}
	if (!bool(*this) || !bool(number))
	{
		return 0_ln;
	}
	if (this->data[0] == '1')
	{
		return sign ? number : -number;
	}
	if (number.data[0] == '1')
	{
		return number.sign ? *this : -*this;
	}

	size_t result_size = size + number.size;
	char *dop = new char[result_size];
	for (size_t i = 0; i < result_size; i++)
	{
		dop[i] = '0';
	}
	for (size_t i = 0; i < size; i++)
	{
		int flag = 0;
		int digit;
		for (size_t j = 0; j < number.size; j++)
		{
			digit = (data[i] - '0') * (j < number.size ? (number.data[j] - '0') : 0);
			digit += dop[j + i] - '0' + flag;
			flag = digit / 10;

			dop[j + i] = (char)(digit % 10 + '0');
		}

		if (flag > 0)
		{
			digit = dop[number.size + i] - '0' + flag;
			dop[number.size + i] = (char)(digit % 10 + '0');
		}
	}

	char *answer = new char[result_size + 1];
	for (size_t i = 0; i < result_size; i++)
	{
		answer[i] = dop[result_size - 1 - i];
	}
	answer[result_size] = '\0';
	delete[] dop;

	LN result(answer);
	delete[] answer;

	return sign == number.sign ? result : -result;
}

LN &LN::operator*=(const LN &number)
{
	*this = *this * number;
	return *this;
}

LN &LN::operator+=(const LN &number)
{
	*this = *this + number;
	return *this;
}

LN &LN::operator-=(const LN &number)
{
	*this = *this - number;
	return *this;
}

bool LN::operator==(const LN &number) const
{
	if (is_nan || number.is_nan)
	{
		return false;
	}
	if (sign != number.sign || size != number.size)
	{
		return false;
	}

	for (size_t i = 0; i < size; i++)
	{
		if (data[i] != number.data[i])
		{
			return false;
		}
	}
	return true;
}

bool LN::operator!=(const LN &number) const
{
	return !(*this == number);
}

bool LN::operator<(const LN &number) const
{
	if (is_nan || number.is_nan)
	{
		return false;
	}
	if (sign > number.sign || (sign && number.sign && (size > number.size)) || (!sign && !number.sign && (size < number.size)))
	{
		return false;
	}

	if (sign < number.sign || (sign && number.sign && (size < number.size)) || (!sign && !number.sign && (size > number.size)))
	{
		return true;
	}

	for (size_t i = size; i > 0; i--)
	{
		if (data[i - 1] < number.data[i - 1])
		{
			return sign;
		}
		else if (data[i - 1] > number.data[i - 1])
		{
			return !sign;
		}
	}

	return false;
}

bool LN::operator<=(const LN &number) const
{
	return (*this < number) || (*this == number);
}

bool LN::operator>(const LN &number) const
{
	return !(*this <= number);
}

bool LN::operator>=(const LN &number) const
{
	return !(*this < number);
}

std::ostream &operator<<(std::ostream &out, const LN &number)
{
	if (number.is_nan)
	{
		return out << "NaN\n";
	}
	if (number.sign == 0)
	{
		out << '-';
	}

	for (size_t i = number.size; i > 0; i--)
	{
		out << number.data[i - 1];
	}
	return out << '\n';
}

void LN::delete_data()
{
	delete[] data;
}

char *LN::sum(const char *first, const char *second, size_t first_size, size_t second_size, char *&result, size_t result_size, const int difference)
{
	char *dop;
	dop = new char[result_size];

	int flag = 0;
	int digit;
	if (difference > 0)
	{
		for (size_t i = 0; i < second_size; i++)
		{
			digit = (first[i] - '0') + (second[i] - '0') + flag;
			flag = digit > 9 ? 1 : 0;
			dop[result_size - 1 - i] = (char)(digit + '0' + ((digit > 9) ? (-10) : 0));
		}
		for (size_t i = second_size; i < result_size; i++)
		{
			digit = (first[i] - '0') + flag;
			flag = digit > 9 ? 1 : 0;
			dop[result_size - 1 - i] = (char)(digit + '0' + ((digit > 9) ? (-10) : 0));
		}
	}
	else if (difference < 0)
	{
		for (size_t i = 0; i < first_size; i++)
		{
			digit = (first[i] - '0') + (second[i] - '0') + flag;
			flag = digit > 9 ? 1 : 0;
			dop[result_size - 1 - i] = (char)(digit + '0' + ((digit > 9) ? (-10) : 0));
		}
		for (size_t i = first_size; i < result_size; i++)
		{
			digit = (second[i] - '0') + flag;
			flag = digit > 9 ? 1 : 0;
			dop[result_size - 1 - i] = (char)(digit + '0' + ((digit > 9) ? (-10) : 0));
		}
	}
	else
	{
		for (size_t i = 0; i < result_size; i++)
		{
			digit = (first[i] - '0') + (second[i] - '0') + flag;
			flag = digit > 9 ? 1 : 0;
			dop[result_size - 1 - i] = (char)(digit + '0' + ((digit > 9) ? (-10) : 0));
		}
	}

	if (flag == 1)
	{
		result_size++;
		result = new char[result_size + 1];
		result[0] = '1';
		for (size_t i = 1; i < result_size; i++)
		{
			result[i] = dop[i - 1];
		}
	}
	else
	{
		result = new char[result_size + 1];
		for (size_t i = 0; i < result_size; i++)
		{
			result[i] = dop[i];
		}
	}
	result[result_size] = '\0';
	delete[] dop;

	return result;
}

char *LN::sub(const char *first, const char *second, size_t second_size, char *&result, size_t result_size)
{
	char *dop;
	dop = new char[result_size];
	int flag = 0;
	for (size_t i = 0; i < second_size; i++)
	{
		int digit = (first[i] - '0') - (second[i] - '0') - flag;
		flag = digit < 0 ? 1 : 0;
		dop[result_size - 1 - i] = (char)(digit + '0' + (flag ? 10 : 0));
	}
	for (size_t i = second_size; i < result_size; i++)
	{
		int digit = (first[i] - '0') - flag;
		flag = digit < 0 ? 1 : 0;
		dop[result_size - 1 - i] = (char)(digit + '0' + (flag ? 10 : 0));
	}

	int zero_count = 0;
	size_t i = 0;
	while (dop[i] == '0')
	{
		zero_count++;
		i++;
	}

	result = new char[result_size - zero_count + 1];
	for (i = 0; i < result_size - zero_count; i++)
	{
		result[i] = dop[i + zero_count];
	}
	result[result_size - zero_count] = '\0';
	delete[] dop;

	return result;
}
