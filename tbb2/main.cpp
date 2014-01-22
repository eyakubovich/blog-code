#include <stdio.h>
#include <string>

#include <tbb/flow_graph.h>
#include <boost/proto/proto.hpp>

#include "edsl.h"

struct body {
	std::string my_name;
	body( const char *name ) : my_name(name) {}
	void operator()( tbb::flow::continue_msg ) const {
		printf("%s\n", my_name.c_str());
	}
};

int main() {
	tbb::flow::graph g;
		 
	bcast start(g);
	cont a = cont_node( g, body("A"));
	cont b = cont_node( g, body("B"));
	cont c = cont_node( g, body("C"));
	cont d = cont_node( g, body("D"));
	cont e = cont_node( g, body("E"));

	auto expr =
		start > (a > e + c)
			  + (b > c > d);

	grammar()(expr);

	boost::proto::value(start).try_put( tbb::flow::continue_msg() );
	g.wait_for_all();

	return 0;
}

