#pragma once

#include <memory>
#include <utility>

struct shared_ownership {
	template <typename T> using ptr_t = std::shared_ptr<T>;
};

struct unique_ownership {
	template <typename T> using ptr_t = std::unique_ptr<T>;
};

template <typename T, typename... Args>
std::unique_ptr<T> make_with_ownership(unique_ownership,  Args... args) {
	// until we have make_unique in C+14
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename T, typename... Args>
std::shared_ptr<T> make_with_ownership(shared_ownership,  Args... args) {
	return std::make_shared<T>(std::forward<Args>(args)...);
}

