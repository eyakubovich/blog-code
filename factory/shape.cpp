#include "make_with_ownership.h"

struct shape {
	virtual ~shape() {}
};

struct ellipse : shape {
	ellipse(int rad_a, int rad_b) {}
};

struct triangle : shape {
	triangle(int base, int height) {}
};

struct rectangle : shape {
	rectangle(int width, int height) {}
};

enum class shape_type { ellipse, triangle, rectangle };

struct rect {
	int w, h;
};

template <typename OwnTag>
typename OwnTag::template ptr_t<shape> make_shape(shape_type type, rect bounds, OwnTag owntag) {
	switch( type ) {
		case shape_type::ellipse:
			return make_with_ownership<ellipse>(owntag, bounds.w / 2, bounds.h / 2);
		case shape_type::triangle:
			return make_with_ownership<triangle>(owntag, bounds.w, bounds.h);
		case shape_type::rectangle:
			return make_with_ownership<rectangle>(owntag, bounds.w, bounds.h);
	}
}

inline std::unique_ptr<shape> make_unique_shape(shape_type type, rect bounds) {
	return make_shape(type, bounds, unique_ownership());
}

inline std::shared_ptr<shape> make_shared_shape(shape_type type, rect bounds) {
	return make_shape(type, bounds, shared_ownership());
}

int main() {
	auto p = make_unique_shape(shape_type::ellipse, { 3, 5 });
	auto q = make_shared_shape(shape_type::rectangle, { 4, 6 });

	return 0;
}

