#pragma once

using boost::adaptors::filtered;
using boost::adaptors::transformed;

template <typename F>
struct expander {
	F f;

	template <typename... Args>
	typename std::result_of<F(Args...)>::type operator()(std::tuple<Args...> tup) const {
		return invoke(f, tup);
	}
};

// --- xfiltered ---
template< class T >
struct xfilter_holder : boost::range_detail::holder<T> {
	xfilter_holder( T r ) : boost::range_detail::holder<T>(r) { }
};

template< class InputRng, class Pred >
inline boost::filtered_range<expander<Pred>, const InputRng>
operator|( const InputRng& r, const xfilter_holder<Pred>& f ) {
	return boost::filtered_range<expander<Pred>, const InputRng>( expander<Pred>{f.val}, r );
}

const boost::range_detail::forwarder<xfilter_holder> xfiltered = boost::range_detail::forwarder<xfilter_holder>();

// --- xtransformed---
template< class T >
struct xtransform_holder : boost::range_detail::holder<T> {
	xtransform_holder( T r ) : boost::range_detail::holder<T>(r) { }
};

template< class InputRng, class F >
inline boost::transformed_range<expander<F>, const InputRng>
operator|( const InputRng& r, const xtransform_holder<F>& f ) {
	return boost::transformed_range<expander<F>, const InputRng>( expander<F>{f.val}, r );
}

const boost::range_detail::forwarder<xtransform_holder> xtransformed = boost::range_detail::forwarder<xtransform_holder>();
