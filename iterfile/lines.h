#pragma once

#include <utility>
#include <fstream>

struct line_t : std::string { 
	 friend std::istream& operator>>(std::istream& is, line_t& line) {
		 return std::getline(is, line);
	 }
};

typedef std::istream_iterator<line_t> line_iterator;

template <typename T>
class line_range {
	T istr;
	line_iterator b;

public:
	typedef line_iterator iterator;
	typedef line_iterator const_iterator;

	line_range(T&& is) :
		istr(std::forward<T>(is)),
		b(istr)
   	{}

	line_range(line_range&&) = default;
	line_range& operator=(line_range&&) = default;

	line_iterator begin() const { return b; }
	line_iterator end() const { return line_iterator(); }
};

template <typename S>
auto lines(S&& is) -> decltype(line_range<S>(std::forward<S>(is))) {
	return line_range<S>(std::forward<S>(is));
}
