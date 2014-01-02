#pragma once

#include <functional>

template <typename F>
class deferer {
	friend class deferred;

	F fun;
	bool enabled;

public:
	deferer(F const& f) : fun(f), enabled(true) {}

	deferer(deferer<F>&& rhs) : fun(rhs.fun), enabled(rhs.enabled) {
		rhs.enabled = false;
	}

	deferer<F>& operator=(deferer<F>&& rhs) {
		if( this != &rhs ) {
			fun = rhs.fun;
			enabled = rhs.enabled;
			rhs.enabled = false;
		}
		return *this;
	}

	deferer(deferer<F> const& ) = delete;
	deferer<F>& operator=(deferer<F> const&) = delete;
	
	~deferer() {
		if( enabled )
			fun();
	}

	void cancel() {
		enabled = false;
	}
};

class deferred {
	std::function<void ()> fun;

public:
	deferred() = default;
	deferred(deferred const&) = delete;
	deferred& operator=(deferred const&) = delete;

	template <typename F>
	deferred(deferer<F>&& d) {
		if( d.enabled ) {
			fun = d.fun;
			d.enabled = false;
		}
	}

	template <typename F>
	deferred& operator=(deferer<F>&& d) {
		fun = d.fun;
		return *this;
	}

	~deferred() {
		if( fun )
			fun();
	}
};

template <typename F>
deferer<F> defer(F const& f) {
	return deferer<F>{ f };
}

