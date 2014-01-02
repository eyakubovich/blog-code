#include <stdio.h>
#include <string>
#include <set>
#include <type_traits>

#include <tbb/flow_graph.h>

#include <boost/proto/proto.hpp>

#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/as_vector.hpp>
#include <boost/fusion/include/join.hpp>
#include <boost/fusion/include/for_each.hpp>


namespace flow = tbb::flow;
namespace proto = boost::proto;
namespace fusion = boost::fusion;

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

template <typename Senders, typename Receivers>
struct compound_node {
	typedef Senders senders_t;
	typedef Receivers receivers_t;

	senders_t senders;
	receivers_t receivers;
};

template <typename S, typename R>
compound_node<S, R> mk_compound_node(S s, R r) {
	return compound_node<S, R>{s, r};
}

struct join : proto::callable {
	template <typename Sig>
	struct result;

	template <typename This, typename LeftNode, typename RightNode>
	struct result<This(LeftNode, RightNode)> {
		typedef typename LeftNode::senders_t left_senders_t;
		typedef typename LeftNode::receivers_t left_receivers_t;
		typedef typename RightNode::senders_t right_senders_t;
		typedef typename RightNode::receivers_t right_receivers_t;

		typedef compound_node<
			typename fusion::result_of::as_vector<
				typename fusion::result_of::join<left_senders_t const, right_senders_t const>::type
			>::type,
			typename fusion::result_of::as_vector<
				typename fusion::result_of::join<left_receivers_t const, right_receivers_t const>::type
			>::type
		> type;
	};

	template <typename LeftNode, typename RightNode>
	typename result<join(LeftNode, RightNode)>::type operator()(LeftNode left, RightNode right) const {
		return mk_compound_node(
				fusion::as_vector(fusion::join(left.senders, right.senders)),
				fusion::as_vector(fusion::join(left.receivers, right.receivers))
			);
	};
};

//void make_edge(flow::sender<flow::continue_msg>&, flow::receiver<flow::continue_msg>&);

struct splice : proto::callable {
	template <typename Sig>
	struct result;

	template <typename This, typename LeftNode, typename RightNode>
	struct result<This(LeftNode, RightNode)> {
		typedef compound_node<
			typename RightNode::senders_t,
			typename LeftNode::receivers_t
		> type;
	};

	template <typename LeftNode, typename RightNode>
	typename result<splice(LeftNode, RightNode)>::type operator()(LeftNode left, RightNode right) const {
		fusion::for_each(left.senders, [&](flow::sender<flow::continue_msg>* s) {
			fusion::for_each(right.receivers, [=](flow::receiver<flow::continue_msg>* r) {
				make_edge(*s, *r);
			});
		});

		return mk_compound_node(right.senders, left.receivers);
	}
};

struct make_compound_node : proto::callable {

	typedef compound_node<
			fusion::vector<flow::sender<flow::continue_msg> *>,
			fusion::vector<flow::receiver<flow::continue_msg> *>
		> result_type;

	result_type	operator()(flow::sender<flow::continue_msg>& s, flow::receiver<flow::continue_msg>& r) {
		return mk_compound_node(
				fusion::vector<flow::sender<flow::continue_msg> *>(&s),
				fusion::vector<flow::receiver<flow::continue_msg> *>(&r));
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

