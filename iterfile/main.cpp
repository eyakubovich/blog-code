#include <iostream>
#include <boost/range/algorithm/for_each.hpp>

#include "lines.h"

int main()
{
	std::ifstream file("somefile.txt");
	for( auto& line: lines(file) )
		std::cout << line << std::endl;

	for( auto& line: lines(std::ifstream("somefile.txt")) )
		std::cout << line << std::endl;


	std::ifstream file2("somefile.txt");
	boost::for_each(lines(file2), [](std::string const& line) {
		std::cout << line << std::endl;
	});

	boost::for_each(lines(std::ifstream("somefile.txt")), [](std::string const& line) {
		std::cout << line << std::endl;
	});

    return 0;
}

