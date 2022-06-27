#include "LN.h"
#include "return_codes.h"

#include <iostream>
#include <stack>
#include <string>

std::stack< LN > stack;

void get1(LN& a)
{
	a = stack.top();
	stack.pop();
}

void get2(LN& a, LN& b)
{
	b = stack.top();
	stack.pop();
	a = stack.top();
	stack.pop();
}

void printStack()
{
	while (!stack.empty())
	{
		std::cout << stack.top();
		stack.pop();
	}
}

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		std::cerr << "Wrong number of arguments (not 3): " << argc << "\n";
		return ERROR_INVALID_PARAMETER;
	}

	if (!freopen(argv[1], "r", stdin))
	{
		std::cerr << "Can't find or open input file «" << argv[1] << "»\n";
		return ERROR_FILE_NOT_FOUND;
	}

	std::string str;
	LN number1;
	LN number2;
	while (getline(std::cin, str))
	{
		switch (str[0])
		{
		case '+':
			get2(number1, number2);
			try
			{
				stack.push(number1 + number2);
			} catch (const std::bad_alloc& e)
			{
				std::cerr << "Can't allocate memory for +" << std::endl;
				fclose(stdin);

				return ERROR_OUTOFMEMORY;
			}

			break;
		case '-':
			if (str[1] == 'N' || (str[1] >= '0' && str[1] <= '9'))
			{
				char* c_number = const_cast< char* >(str.erase(0, 1).c_str());

				try
				{
					stack.push(-LN(c_number));
				} catch (const std::bad_alloc& e)
				{
					std::cerr << "Can't allocate memory for create LN" << std::endl;
					fclose(stdin);

					return ERROR_MEMORY;
				}
			}
			else
			{
				get2(number1, number2);
				try
				{
					stack.push(number1 - number2);
				} catch (const std::bad_alloc& e)
				{
					fclose(stdin);
					std::cerr << "Can't allocate memory for -" << std::endl;

					return ERROR_OUTOFMEMORY;
				}
			}

			break;
		case '*':
			get2(number1, number2);
			try
			{
				stack.push(number1 * number2);
			} catch (const std::bad_alloc& e)
			{
				fclose(stdin);
				std::cerr << "Can't allocate memory for *" << std::endl;

				return ERROR_OUTOFMEMORY;
			}

			break;
		case '/':
			try
			{
				/*get2(number1, number2);
				stack.push(number1 / number2);*/
			} catch (const std::bad_alloc& e)
			{
				fclose(stdin);
				std::cerr << "Can't allocate memory for /" << std::endl;

				return ERROR_OUTOFMEMORY;
			}

			break;
		case '%':
			try
			{
				/*get2(number1, number2);
				stack.push(number1 % number2);*/
			} catch (const std::bad_alloc& e)
			{
				fclose(stdin);
				std::cerr << "Can't allocate memory for %" << std::endl;

				return ERROR_OUTOFMEMORY;
			}

			break;
		case '~':
			try
			{
				/*get1(number1);
				stack.push(~number1);*/
			} catch (const std::bad_alloc& e)
			{
				fclose(stdin);
				std::cerr << "Can't allocate memory for ~" << std::endl;

				return ERROR_OUTOFMEMORY;
			}

			break;
		case '_':
			get1(number1);
			stack.push(-number1);

			break;
		case '<':
			get2(number1, number2);
			try
			{
				stack.push((str[1] == '=') ? ((number1 <= number2) ? 1_ln : 0_ln) : ((number1 < number2) ? 1_ln : 0_ln));
			} catch (const std::bad_alloc& e)
			{
				fclose(stdin);
				std::cerr << "Can't allocate memory" << std::endl;

				return ERROR_MEMORY;
			}

			break;
		case '>':
			get2(number1, number2);
			try
			{
				stack.push((str[1] == '=') ? ((number1 >= number2) ? 1_ln : 0_ln) : ((number1 > number2) ? 1_ln : 0_ln));
			} catch (const std::bad_alloc& e)
			{
				fclose(stdin);
				std::cerr << "Can't allocate memory" << std::endl;

				return ERROR_MEMORY;
			}

			break;
		case '!':
			get2(number1, number2);
			try
			{
				stack.push((number1 != number2) ? 1_ln : 0_ln);
			} catch (const std::bad_alloc& e)
			{
				fclose(stdin);
				std::cerr << "Can't allocate memory" << std::endl;

				return ERROR_MEMORY;
			}

			break;
		case '=':
			get2(number1, number2);
			try
			{
				stack.push((number1 == number2) ? 1_ln : 0_ln);
			} catch (const std::bad_alloc& e)
			{
				fclose(stdin);
				std::cerr << "Can't allocate memory" << std::endl;

				return ERROR_MEMORY;
			}

			break;
		default:
			try
			{
				char* c_number = const_cast< char* >(str.c_str());
				stack.push(LN(c_number));
			} catch (const std::bad_alloc& e)
			{
				fclose(stdin);
				std::cerr << "Can't allocate memory" << std::endl;

				return ERROR_MEMORY;
			}
		}
	}
	fclose(stdin);

	if (!freopen(argv[2], "w", stdout))
	{
		std::cerr << "Can't create or open output file «" << argv[2] << "»\n";
		return ERROR_FILE_NOT_FOUND;
	}
	printStack();
	fclose(stdout);

	return 0;
}