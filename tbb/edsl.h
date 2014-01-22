#pragma once

#include <set>
#include <boost/proto/proto.hpp>
#include <tbb/flow_graph.h>

namespace flow = tbb::flow;
namespace proto = boost::proto;

typedef tbb::flow::broadcast_node<tbb::flow::continue_msg> bcast_node;
typedef tbb::flow::continue_node<tbb::flow::continue_msg> cont_node;
typedef boost::proto::literal<bcast_node> bcast;
typedef boost::proto::literal<cont_node> cont;

typedef std::set<tbb::flow::sender<tbb::flow::continue_msg>*> sender_set;
typedef std::set<tbb::flow::receiver<tbb::flow::continue_msg>*> receiver_set;
typedef std::tuple<sender_set, receiver_set> compound_node;

// node that gets formed using +
struct join : boost::proto::callable {
	typedef compound_node result_type;

	compound_node operator()(compound_node left, compound_node right) const {
		sender_set& senders = std::get<0>(left);
		senders.insert(std::get<0>(right).begin(), std::get<0>(right).end());

		receiver_set& receivers = std::get<1>(left);
		receivers.insert(std::get<1>(right).begin(), std::get<1>(right).end());

		return compound_node(std::move(senders), std::move(receivers));
	};
};

// node that gets formed using >
struct splice : boost::proto::callable {
	typedef compound_node result_type;

	compound_node operator()(compound_node left, compound_node right) const {
		for( auto s: std::get<0>(left) ) {
			for( auto r: std::get<1>(right) ) {
				tbb::flow::make_edge(*s, *r);
			}
		}

		sender_set& senders = std::get<0>(right);
		receiver_set& receivers = std::get<1>(left);

		return compound_node(std::move(senders), std::move(receivers));
	}
};

struct make_compound_node : boost::proto::callable {
	typedef compound_node result_type;

	compound_node operator()(tbb::flow::sender<tbb::flow::continue_msg>& s, tbb::flow::receiver<tbb::flow::continue_msg>& r) {
		return compound_node({&s}, {&r});
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

