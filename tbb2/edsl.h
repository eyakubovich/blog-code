#pragma once

#include <type_traits>

#include <tbb/flow_graph.h>

#include <boost/proto/proto.hpp>

#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/as_vector.hpp>
#include <boost/fusion/include/join.hpp>
#include <boost/fusion/include/for_each.hpp>

typedef tbb::flow::broadcast_node<tbb::flow::continue_msg> bcast_node;
typedef tbb::flow::continue_node<tbb::flow::continue_msg> cont_node;
typedef boost::proto::literal<bcast_node> bcast;
typedef boost::proto::literal<cont_node> cont;

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

// node that gets formed using +
struct join : boost::proto::callable {
	template <typename Sig>
	struct result;

	template <typename This, typename LeftNode, typename RightNode>
	struct result<This(LeftNode, RightNode)> {
		typedef typename LeftNode::senders_t left_senders_t;
		typedef typename LeftNode::receivers_t left_receivers_t;
		typedef typename RightNode::senders_t right_senders_t;
		typedef typename RightNode::receivers_t right_receivers_t;

		typedef compound_node<
			typename boost::fusion::result_of::as_vector<
				typename boost::fusion::result_of::join<
					left_senders_t const,
					right_senders_t const
				>::type
			>::type,
			typename boost::fusion::result_of::as_vector<
				typename boost::fusion::result_of::join<
					left_receivers_t const,
					right_receivers_t const
				>::type
			>::type
		> type;
	};

	template <typename LeftNode, typename RightNode>
	typename result<join(LeftNode, RightNode)>::type operator()(LeftNode left, RightNode right) const {
		return mk_compound_node(
				boost::fusion::as_vector(boost::fusion::join(left.senders, right.senders)),
				boost::fusion::as_vector(boost::fusion::join(left.receivers, right.receivers))
			);
	};
};

//void make_edge(tbb::flow::sender<tbb::flow::continue_msg>&, tbb::flow::receiver<tbb::flow::continue_msg>&);

// node that gets formed using >
struct splice : boost::proto::callable {
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
		boost::fusion::for_each(left.senders, [&](tbb::flow::sender<tbb::flow::continue_msg>* s) {
			boost::fusion::for_each(right.receivers, [=](tbb::flow::receiver<tbb::flow::continue_msg>* r) {
				make_edge(*s, *r);
			});
		});

		return mk_compound_node(right.senders, left.receivers);
	}
};

struct make_compound_node : boost::proto::callable {

	typedef compound_node<
			boost::fusion::vector<tbb::flow::sender<tbb::flow::continue_msg> *>,
			boost::fusion::vector<tbb::flow::receiver<tbb::flow::continue_msg> *>
		> result_type;

	result_type	operator()(tbb::flow::sender<tbb::flow::continue_msg>& s, tbb::flow::receiver<tbb::flow::continue_msg>& r) {
		return mk_compound_node(
				boost::fusion::vector<tbb::flow::sender<tbb::flow::continue_msg> *>(&s),
				boost::fusion::vector<tbb::flow::receiver<tbb::flow::continue_msg> *>(&r));
	}
};

struct grammar
	: boost::proto::or_<
		boost::proto::when<
			boost::proto::plus<grammar, grammar>,
			join(grammar(boost::proto::_left), grammar(boost::proto::_right))
		>,

		boost::proto::when<
			boost::proto::greater<grammar, grammar>,
			splice(grammar(boost::proto::_left), grammar(boost::proto::_right))
		>,

		boost::proto::when<
			bcast,
			make_compound_node(boost::proto::_value, boost::proto::_value)
		>,

		boost::proto::when<
			cont, make_compound_node(boost::proto::_value, boost::proto::_value)
		>
	>
{};

