#include "phonebook.h"

std::istream& operator>>(std::istream& in, Phonebook& pb)
{
	in >> pb.lastname >> pb.name >> pb.patronymic >> pb.number;
	return in;
}

std::ostream& operator<<(std::ofstream& out, const Phonebook& pb)
{
	out << pb.lastname << " " << pb.name << " " << pb.patronymic << " " << pb.number;
	return out;
}

bool operator<(const Phonebook& pb1, const Phonebook& pb2)
{
	if (pb1.lastname != pb2.lastname)
	{
		return pb1.lastname < pb2.lastname;
	}
	else if (pb1.name != pb2.name)
	{
		return pb1.name < pb2.name;
	}
	else if (pb1.patronymic != pb2.patronymic)
	{
		return pb1.patronymic < pb2.patronymic;
	}
	else
	{
		return pb1.number < pb2.number;
	}
}
