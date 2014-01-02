#include <stdio.h>
#include <string>
#include <set>
#include <boost/proto/proto.hpp>
#include <tbb/flow_graph.h>

namespace flow = tbb::flow;
namespace proto = boost::proto;

struct body {
	std::string my_name;
	body( const char *name ) : my_name(name) {}
	void operator()( flow::continue_msg ) const {
		printf("%s\n", my_name.c_str());
	}
};

typedef flow::broadcast_node<flow::continue_msg> bcast_node;
typedef flow::continue_node<flow::continue_msg> cont_node;
typedef proto::literal<bcast_node> bcast;
typedef proto::literal<cont_node> cont;

typedef std::set<flow::sender<flow::continue_msg>*> sender_set;
typedef std::set<flow::receiver<flow::continue_msg>*> receiver_set;
typedef std::tuple<sender_set, receiver_set> compound_node;

struct join : proto::callable {
	typedef compound_node result_type;

	compound_node operator()(compound_node left, compound_node right) const {
		sender_set& senders = std::get<0>(left);
		senders.insert(std::get<0>(right).begin(), std::get<0>(right).end());

		receiver_set& receivers = std::get<1>(left);
		receivers.insert(std::get<1>(right).begin(), std::get<1>(right).end());

		return compound_node(std::move(senders), std::move(receivers));
	};
};

struct splice : proto::callable {
	typedef compound_node result_type;

	compound_node operator()(compound_node left, compound_node right) const {
		for( auto s: std::get<0>(left) )
			for( auto r: std::get<1>(right) )
				flow::make_edge(*s, *r);

		sender_set& senders = std::get<0>(right);
		receiver_set& receivers = std::get<1>(left);

		return compound_node(std::move(senders), std::move(receivers));
	}
};

struct make_compound_node : proto::callable {
	typedef compound_node result_type;

	compound_node operator()(flow::sender<flow::continue_msg>& s, flow::receiver<flow::continue_msg>& r) {
		return compound_node({&s}, {&r});
	}
};

struct grammar
	: proto::or_<
		proto::when<proto::plus<grammar, grammar>, join(grammar(proto::_left), grammar(proto::_right))>,
		proto::when<proto::greater<grammar, grammar>, splice(grammar(proto::_left), grammar(proto::_right))>,
		proto::when<bcast, make_compound_node(proto::_value, proto::_value)>,
		proto::when<cont, make_compound_node(proto::_value, proto::_value)>
	>
{};


int main() {
	flow::graph g;
		 
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

	proto::value(start).try_put( flow::continue_msg() );
	g.wait_for_all();

	return 0;
}

