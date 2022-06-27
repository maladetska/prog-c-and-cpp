#pragma once

#include <fstream>
#include <string>

class Phonebook
{
  public:
	std::string lastname;
	std::string name;
	std::string patronymic;
	long long number;
};

std::istream& operator>>(std::istream&, Phonebook&);
std::ostream& operator<<(std::ofstream&, const Phonebook&);
bool operator<(const Phonebook&, const Phonebook&);