#pragma once

#include <tuple>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/range.hpp>
#include "invoke.h"

template <typename R>
struct range_reference {
	typedef typename boost::range_iterator<R>::type iter;
	typedef typename iter::reference type;
};

template <typename... Rs>
struct value_type {
	typedef std::tuple<
		typename range_reference<Rs>::type...
	> type;
};

template <size_t N>
using const_int = std::integral_constant<size_t, N>;

template <typename... Rs>
class cartesian_iterator : public boost::iterator_facade<
	cartesian_iterator<Rs...>,  // pass self per CRTP
	typename value_type<Rs...>::type,  // value_type
	boost::forward_traversal_tag,  // iterator category
	typename value_type<Rs...>::type  // reference -- same as value_type!
>
{
	std::tuple<typename boost::range_iterator<Rs>::type... > iters;
	std::tuple<Rs&...> ranges;

public:
	cartesian_iterator() {}

	cartesian_iterator(Rs&... rs) : ranges(rs...), iters(boost::begin(rs)...)
	{
	}

	cartesian_iterator(Rs&... rs, int) :
		ranges(rs...),
		iters(boost::end(rs)...)
	{
	}

	void increment() {
		increment(const_int<sizeof...(Rs) - 1>());
	}

	bool equal(cartesian_iterator const& other ) const { return iters == other.iters; }

	typename value_type<Rs...>::type dereference() const {
		return dereference(tuple_indices(iters));
	}

private:
	template <size_t N>
	bool increment(const_int<N>) {
		if( ++(std::get<N>(iters)) == boost::end(std::get<N>(ranges)) ) {
			if( !increment(const_int<N-1>()) )
				return false;
			std::get<N>(iters) = boost::begin(std::get<N>(ranges));
		}
		return true;
	}

	bool increment(const_int<0>) {
		return ++(std::get<0>(iters)) != boost::end(std::get<0>(ranges));
	}

	template <size_t... Indices>
	typename value_type<Rs...>::type dereference(seq<Indices...>) const {
		typedef typename value_type<Rs...>::type result_t;
		return result_t(*std::get<Indices>(iters)...);
	}
};

template <typename... Rs>
using cartesian_range = boost::iterator_range<cartesian_iterator<Rs...>>;

template <typename... Rs>
typename cartesian_range<Rs...>::type cartesian(Rs&... rs) {
	typedef cartesian_iterator<Rs...> iter_t;
	return cartesian_range<Rs...>(iter_t(rs...), iter_t(rs..., 0));
}

