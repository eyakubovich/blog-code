#include <vector>
#include <iostream>
#include <type_traits>
#include <boost/range/counting_range.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/utility/result_of.hpp>
#include "cartesian.h"
#include "adaptors.h"

int main() {

	std::vector<int> xx = { 1, 2, 3 };
	std::vector<char> yy = { 'a', 'b', 'c' };
	std::vector<double> zz = { 0.1, 0.2, 0.3 };
/*
	auto r = cartesian(xx, yy, zz)
	       | filtered([](std::tuple<int&, char&, double&> x) { return std::get<0>(x) > 1 && std::get<1>(x) < 'c'; })
	       | transformed([](std::tuple<int&, char&, double&> x) { return std::get<0>(x) + int(std::get<1>(x)) + std::get<2>(x); });

*/
	auto r = cartesian(xx, yy, zz)
	       | xfiltered([](int x, char y, double z) { return x > 1 && y < 'c'; })
	       | xtransformed([](int x, char y, double z) { return x + int(y) + z; });

	for( auto x : r )
		std::cout << x << std::endl;

	return 0;
}

