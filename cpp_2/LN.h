#pragma once

#include <string_view>	  // for constructor

#include <cstring>
#include <iostream>	   // for error log

class LN;

LN operator"" _ln(const char *);

class LN
{
  public:
	LN(const LN &number);
	LN(LN &&number) noexcept;

	explicit LN(long long = 0);
	explicit LN(const char *number);

	[[maybe_unused]] explicit LN(const std::string_view &number);

	~LN();

	LN &operator=(const LN &number);
	LN &operator=(LN &&number) noexcept;

	LN operator-() const;

	LN operator+(const LN &number) const;
	LN operator-(const LN &number) const;
	LN operator*(const LN &number) const;

	LN &operator+=(const LN &number);
	LN &operator-=(const LN &number);
	LN &operator*=(const LN &number);

	[[nodiscard]] bool operator==(const LN &number) const;
	[[nodiscard]] bool operator!=(const LN &number) const;
	[[nodiscard]] bool operator<(const LN &number) const;
	[[nodiscard]] bool operator<=(const LN &number) const;
	[[nodiscard]] bool operator>(const LN &number) const;
	[[nodiscard]] bool operator>=(const LN &number) const;

	[[nodiscard]] explicit operator long long() const noexcept(false);
	[[nodiscard]] explicit operator bool() const;

	friend std::ostream &operator<<(std::ostream &, const LN &);

  private:
	void delete_data();
	/*
	 * first > 0 && second > 0
	 */
	[[nodiscard]] static char *
		sum(const char *first, const char *second, size_t first_size, size_t second_size, char *&result, size_t result_size, int difference);
	/*
	 * first > 0 && second > 0 && first > second
	 */
	[[nodiscard]] static char *sub(const char *first, const char *second, size_t second_size, char *&result, size_t result_size);
	char *data = nullptr;
	/*
	 * sign of number. If number <= 0 : sign == false, else sign == true
	 */
	bool sign = true;
	/*
	 * digits count of number
	 */
	size_t size = 0;
	bool is_nan = false;
};