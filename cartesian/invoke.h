#include <utility>

// the sequence type
template<size_t...>
struct seq { };

// meta-function to generate seq<0, 1, ..., N-1>
template<size_t N, size_t ...S>
struct gens : gens<N-1, N-1, S...> { };

template<size_t ...S>
struct gens<0, S...> {
	typedef seq<S...> type;
};

// accepts a tuple and returns seq<0, 1, ..., N-1>
template <typename... Ts>
typename gens<sizeof...(Ts)>::type tuple_indices(std::tuple<Ts...> const&) {
	return typename gens<sizeof...(Ts)>::type();
};

// helper that captures indices into a parameter pack and invokes f
template<typename F, typename Args, size_t ...Indices>
auto call_func(F&& f, Args&& args, seq<Indices...>) -> decltype(f(std::get<Indices>(args)...)) {
	return f(std::get<Indices>(args)...);
}

// takes function f and tuple of args and invokes f with args
template <typename F, typename Args>
auto invoke(F&& f, Args&& args) -> decltype(call_func(std::forward<F>(f), std::forward<Args>(args), tuple_indices(args))) {
	return call_func(std::forward<F>(f), std::forward<Args>(args), tuple_indices(args));
}

