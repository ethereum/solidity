/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file net.cpp
 * @author Alex Leverington <nessence@gmail.com>
 * @date 2014
 */

#include <boost/test/unit_test.hpp>
#include <libdevcore/Worker.h>
#include <libdevcrypto/Common.h>
#include <libp2p/UDP.h>
using namespace std;
using namespace dev;
using namespace dev::p2p;
namespace ba = boost::asio;
namespace bi = ba::ip;

/**
 * Ping packet: Check if node is alive.
 * PingNode is cached and regenerated after expiration - t, where t is timeout.
 *
 * signature: Signature of message.
 * ipAddress: Our IP address.
 * port: Our port.
 * expiration: Triggers regeneration of packet. May also provide control over synchronization.
 *
 * Ping is used to implement evict. When a new node is seen for
 * a given bucket which is full, the least-responsive node is pinged.
 * If the pinged node doesn't respond then it is removed and the new
 * node is inserted.
 */
struct PingNode: RLPDatagram
{
	bytes ipAddress;
	uint16_t port;
	uint64_t expiration;

	Signature signature;
	
//	void streamRLP(RLPStream& _s) const { _s.appendList(3); _s << ipAddress << port << expiration; }
};

struct Pong: RLPDatagram
{
	// todo: weak-signed pong
	Address from;
	uint64_t replyTo;	/// expiration from PingNode
	
	void streamRLP(RLPStream& _s) const { _s.appendList(2); _s << from << replyTo; }
};

/**
 * FindNeighbors Packet: Request k-nodes, closest to the target.
 * FindNeighbors is cached and regenerated after expiration - t, where t is timeout.
 *
 * signature: Signature of message.
 * target: Address of NodeId. The responding node will send back nodes closest to the target.
 * expiration: Triggers regeneration of packet. May also provide control over synchronization.
 *
 */
struct FindNeighbors: RLPDatagram
{
	h160 target;
	uint64_t expiration;
	
	Signature signature;
	
	void streamRLP(RLPStream& _s) const { _s.appendList(2); _s << target << expiration; }
};

/**
 * Node Packet: Multiple node packets are sent in response to FindNeighbors.
 */
struct Neighbors: RLPDatagram
{
	struct Node
	{
		bytes ipAddress;
		uint16_t port;
		NodeId node;
//		void streamRLP(RLPStream& _s) const { _s.appendList(3); _s << ipAddress << port << node; }
	};
	
	std::set<Node> nodes;
	h256 nonce;
	
	Signature signature;
	
//	void streamRLP(RLPStream& _s) const { _s.appendList(2); _s.appendList(nodes.size()); for (auto& n: nodes) n.streamRLP(_s); _s << nonce; }
};

/**
 * NodeTable using S/Kademlia system for node discovery and preference.
 * untouched buckets are refreshed if they have not been touched within an hour
 *
 * Thread-safety is ensured by modifying NodeEntry details via 
 * shared_ptr replacement instead of mutating values.
 *
 * @todo don't try to evict node if node isRequired. (support for makeRequired)
 * @todo optimize (use tree for state (or set w/custom compare for cache))
 * @todo constructor support for m_node, m_secret
 * @todo use s_bitsPerStep for find and refresh/ping
 * @todo exclude bucket from refresh if we have node as peer
 * @todo restore nodes
 */
class NodeTable: UDPSocketEvents, public std::enable_shared_from_this<NodeTable>
{
	using nodeSocket = UDPSocket<NodeTable, 1024>;
	using timePoint = std::chrono::steady_clock::time_point;
	
	static unsigned const s_bucketSize = 16;	// Denoted by k in [Kademlia]. Number of nodes stored in each bucket.
//	const unsigned s_bitsPerStep = 5;			// @todo Denoted by b in [Kademlia]. Bits by which address space will be divided for find responses.
	static unsigned const s_alpha = 3;		// Denoted by \alpha in [Kademlia]. Number of concurrent FindNeighbors requests.
	const unsigned s_findTimout = 300;		// How long to wait between find queries.
//	const unsigned s_siblings = 5;			// @todo Denoted by s in [S/Kademlia]. User-defined by sub-protocols.
	const unsigned s_bucketRefresh = 3600;		// Refresh interval prevents bucket from becoming stale. [Kademlia]
	const unsigned s_bits = sizeof(Address);	// Denoted by n.
	const unsigned s_buckets = 8 * s_bits - 1;
	const unsigned s_evictionCheckInterval = 75;	// Interval by which eviction timeouts are checked.
	const unsigned s_pingTimeout = 500;
	static size_t const s_tableSize = Address::size * 8 - 1; // Address::size
	
public:
	static unsigned dist(Address const& _a, Address const& _b) { u160 d = _a ^ _b; unsigned ret; for (ret = 0; d >>= 1; ++ret) {}; return ret; }
	
protected:
	struct NodeDefaultEndpoint
	{
		NodeDefaultEndpoint(bi::udp::endpoint _udp): udp(_udp) {}
		bi::udp::endpoint udp;
	};
	
	struct NodeEntry
	{
		NodeEntry(Address _id, Public _pubk, bi::udp::endpoint _udp): id(_id), pubk(_pubk), endpoint(NodeDefaultEndpoint(_udp)), distance(0) {}
		NodeEntry(NodeEntry _src, Address _id, Public _pubk, bi::udp::endpoint _udp): id(_id), pubk(_pubk), endpoint(NodeDefaultEndpoint(_udp)), distance(dist(_src.id,_id)) {}
		NodeEntry(NodeEntry _src, Address _id, Public _pubk, NodeDefaultEndpoint _gw): id(_id), pubk(_pubk), endpoint(_gw), distance(dist(_src.id,_id)) {}
		Address id;
		Public pubk;
		NodeDefaultEndpoint endpoint;		///< How we've previously connected to this node. (must match node's reported endpoint)
		const unsigned distance;
		timePoint activePing;
	};
	
	struct NodeBucket
	{
		unsigned distance;
		timePoint modified;
		std::list<std::weak_ptr<NodeEntry>> nodes;
	};
	
	using EvictionTimeout = std::pair<std::pair<Address,timePoint>,Address>;

public:
	NodeTable(ba::io_service& _io):
		m_node(NodeEntry(Address(), Public(), bi::udp::endpoint())),
		m_socket(new nodeSocket(_io, *this, 30300)),
		m_socketPtr(m_socket.get()),
		m_io(_io),
		m_bucketRefreshTimer(m_io),
		m_evictionCheckTimer(m_io)
	{
		for (unsigned i = 0; i < s_buckets; i++)
			m_state[i].distance = i, m_state[i].modified = chrono::steady_clock::now() - chrono::seconds(1);
		doRefreshBuckets(boost::system::error_code());
	}
	
	~NodeTable() {
		m_evictionCheckTimer.cancel();
		m_bucketRefreshTimer.cancel();
		m_socketPtr->disconnect();
	}
	
	void join() { doFindNode(m_node.id); }
	
	std::list<Address> nodes() const
	{
		std::list<Address> nodes;
		Guard l(x_nodes);
		for (auto& i: m_nodes)
			nodes.push_back(i.second->id);
		return std::move(nodes);
	}
	
	NodeEntry operator[](Address _id)
	{
		Guard l(x_nodes);
		return *m_nodes[_id];
	}
	
protected:
	void requestNeighbors(NodeEntry const& _node, Address _target) const
	{
		FindNeighbors p;
		p.target = _target;
		
		p.to = _node.endpoint.udp;
		p.seal(m_secret);
		m_socketPtr->send(p);
	}
	
	/// Dispatches udp requests in order to populate node table to be as close as possible to _node.
	void doFindNode(Address _node, unsigned _round = 0, std::shared_ptr<std::set<std::shared_ptr<NodeEntry>>> _tried = std::shared_ptr<std::set<std::shared_ptr<NodeEntry>>>())
	{
		if (!m_socketPtr->isOpen() || _round == 7)
			return;

		auto nearest = findNearest(_node);
		std::list<std::shared_ptr<NodeEntry>> tried;
		for (unsigned i = 0; i < nearest.size() && tried.size() < s_alpha; i++)
			if (!_tried->count(nearest[i]))
			{
				tried.push_back(nearest[i]);
				requestNeighbors(*nearest[i], _node);
			}
			else
				continue;
		
		while (auto n = tried.front())
		{
			_tried->insert(n);
			tried.pop_front();
		}
		
		auto self(shared_from_this());
		m_evictionCheckTimer.expires_from_now(boost::posix_time::milliseconds(s_findTimout));
		m_evictionCheckTimer.async_wait([this, self, _node, _round, _tried](boost::system::error_code const& _ec)
		{
			if (_ec)
				return;
			doFindNode(_node, _round + 1, _tried);
		});
	}
	
	std::vector<std::shared_ptr<NodeEntry>> findNearest(Address _target)
	{
		// send s_alpha FindNeighbors packets to nodes we know, closest to target
		unsigned head = dist(m_node.id, _target);
		unsigned tail = (head - 1) % (s_tableSize - 1);
		
		// todo: optimize with tree
		std::map<unsigned, std::list<std::shared_ptr<NodeEntry>>> found;
		unsigned count = 0;
		
		// if d is 0, then we roll look forward, if last, we reverse, else, spread from d
		if (head != 0 && tail != s_tableSize)
			while (head != tail && count < s_bucketSize)
			{
				Guard l(x_state);
				for (auto& n: m_state[head].nodes)
					if (auto p = n.lock())
					{
						if (count < s_bucketSize)
							found[dist(_target, p->id)].push_back(p);
						else
							break;
					}
				
				if (count < s_bucketSize && head)
					for (auto& n: m_state[tail].nodes)
						if (auto p = n.lock())
						{
							if (count < s_bucketSize)
								found[dist(_target, p->id)].push_back(p);
							else
								break;
						}
				head++;
				tail = (tail - 1) % (s_tableSize - 1);
			}
		else if (head == 0)
			while (head < s_bucketSize && count < s_bucketSize)
			{
				Guard l(x_state);
				for (auto& n: m_state[head].nodes)
					if (auto p = n.lock())
					{
						if (count < s_bucketSize)
							found[dist(_target, p->id)].push_back(p);
						else
							break;
					}
				head--;
			}
		else if (tail == s_tableSize - 1)
			while (tail > 0 && count < s_bucketSize)
			{
				Guard l(x_state);
				for (auto& n: m_state[tail].nodes)
					if (auto p = n.lock())
					{
						if (count < s_bucketSize)
							found[dist(_target, p->id)].push_back(p);
						else
							break;
					}
				tail--;
			}
		
		std::vector<std::shared_ptr<NodeEntry>> ret;
		for (auto& nodes: found)
			for (auto& n: nodes.second)
				ret.push_back(n);
		return std::move(ret);
	}
	
	void ping(bi::address _address, unsigned _port) const
	{
		PingNode p;
		string ip = m_node.endpoint.udp.address().to_string();
		p.ipAddress = asBytes(ip);
		p.port = m_node.endpoint.udp.port();
//		p.expiration;
		p.seal(m_secret);
		m_socketPtr->send(p);
	}
	
	void ping(NodeEntry* _n) const
	{
		if (_n && _n->endpoint.udp.address().is_v4())
			ping(_n->endpoint.udp.address(), _n->endpoint.udp.port());
	}
	
	void evict(std::shared_ptr<NodeEntry> _leastSeen, std::shared_ptr<NodeEntry> _new)
	{
		if (!m_socketPtr->isOpen())
			return;
		
		Guard l(x_evictions);
		m_evictions.push_back(EvictionTimeout(make_pair(_leastSeen->id,chrono::steady_clock::now()), _new->id));
		if (m_evictions.size() == 1)
			doCheckEvictions(boost::system::error_code());
		
		m_evictions.push_back(EvictionTimeout(make_pair(_leastSeen->id,chrono::steady_clock::now()), _new->id));
		ping(_leastSeen.get());
	}
	
	void noteNode(Public _pubk, bi::udp::endpoint _endpoint)
	{
		Address id = right160(sha3(_pubk));
		std::shared_ptr<NodeEntry> node;
		{
			Guard l(x_nodes);
			auto n = m_nodes.find(id);
			if (n == m_nodes.end())
			{
				m_nodes[id] = std::shared_ptr<NodeEntry>(new NodeEntry(m_node, id, _pubk, _endpoint));
				node = m_nodes[id];
			}
			else
				node = n->second;
		}
		
		noteNode(node);
	}
	
	void noteNode(std::shared_ptr<NodeEntry> _n)
	{
		std::shared_ptr<NodeEntry> contested;
		{
			NodeBucket s = bucket(_n.get());
			Guard l(x_state);
			s.nodes.remove_if([&_n](std::weak_ptr<NodeEntry> n)
			{
				auto p = n.lock();
				if (!p || p == _n)
					return true;
				return false;
			});

			if (s.nodes.size() >= s_bucketSize)
			{
				contested = s.nodes.front().lock();
				if (!contested)
				{
					s.nodes.pop_front();
					s.nodes.push_back(_n);
				}
			}
			else
				s.nodes.push_back(_n);
		}
		
		if (contested)
			evict(contested, _n);
	}
	
	void dropNode(std::shared_ptr<NodeEntry> _n)
	{
		NodeBucket s = bucket(_n.get());
		{
			Guard l(x_state);
			s.nodes.remove_if([&_n](std::weak_ptr<NodeEntry> n) { return n.lock() == _n; });
		}
		Guard l(x_nodes);
		m_nodes.erase(_n->id);
	}
	
	NodeBucket const& bucket(NodeEntry* _n) const
	{
		return m_state[_n->distance];
	}
	
	void onReceived(UDPSocketFace*, bi::udp::endpoint const& _from, bytesConstRef _packet)
	{
		RLP rlp(_packet);
		
		
		// whenever a pong is received, first check if it's in m_evictions, if so, remove it
		Guard l(x_evictions);
	}
	
	void onDisconnected(UDPSocketFace*)
	{
		
	}
	
	void doCheckEvictions(boost::system::error_code const& _ec)
	{
		if (_ec || !m_socketPtr->isOpen())
			return;

		m_evictionCheckTimer.expires_from_now(boost::posix_time::milliseconds(s_evictionCheckInterval));
		auto self(shared_from_this());
		m_evictionCheckTimer.async_wait([this, self](boost::system::error_code const& _ec)
		{
			if (_ec)
				return;
			
			bool evictionsRemain = false;
			std::list<shared_ptr<NodeEntry>> drop;
			{
				Guard l(x_evictions);
				for (auto& e: m_evictions)
					if (chrono::steady_clock::now() - e.first.second > chrono::milliseconds(s_pingTimeout))
					{
						Guard l(x_nodes);
						drop.push_back(m_nodes[e.second]);
					}
				evictionsRemain = m_evictions.size() - drop.size() > 0;
			}
			
			for (auto& n: drop)
				dropNode(n);
			
			if (evictionsRemain)
				doCheckEvictions(boost::system::error_code());
		});
	}
	
	void doRefreshBuckets(boost::system::error_code const& _ec)
	{
		cout << "refreshing buckets" << endl;
		if (_ec)
			return;
		
		// first check if there are any pending evictions

		
		bool connected = m_socketPtr->isOpen();
		bool refreshed = false;
		if (connected)
		{
			Guard l(x_state);
			for (auto& d: m_state)
				if (chrono::steady_clock::now() - d.modified > chrono::seconds(s_bucketRefresh))
					while (!d.nodes.empty())
					{
						auto n = d.nodes.front();
						if (auto p = n.lock())
						{
							refreshed = true;
							ping(p.get());
							break;
						}
						d.nodes.pop_front();
					}
		}

		unsigned nextRefresh = connected ? (refreshed ? 200 : s_bucketRefresh*1000) : 10000;
		auto runcb = [this](boost::system::error_code const& error) -> void { doRefreshBuckets(error); };
		m_bucketRefreshTimer.expires_from_now(boost::posix_time::milliseconds(nextRefresh));
		m_bucketRefreshTimer.async_wait(runcb);
	}
	
private:
	NodeEntry m_node;										///< This node.
	Secret m_secret;											///< This nodes secret key.

	mutable Mutex x_nodes;									///< Mutable for thread-safe copy in nodes() const.
	std::map<Address, std::shared_ptr<NodeEntry>> m_nodes;		///< Address -> Node table (most common lookup path)

	Mutex x_state;
	std::array<NodeBucket, s_tableSize> m_state;				///< State table; logbinned nodes.

	Mutex x_evictions;
	std::deque<EvictionTimeout> m_evictions;					///< Eviction timeouts.
	
	shared_ptr<nodeSocket> m_socket;							///< Shared pointer for our UDPSocket; ASIO requires shared_ptr.
	nodeSocket* m_socketPtr;									///< Set to m_socket.get().
	ba::io_service& m_io;										///< Used by bucket refresh timer.
	boost::asio::deadline_timer m_bucketRefreshTimer;			///< Timer which schedules and enacts bucket refresh.
	boost::asio::deadline_timer m_evictionCheckTimer;			///< Timer for handling node evictions.
};

/**
 * Only used for testing. Not useful beyond tests.
 */
class TestHost: public Worker
{
public:
	TestHost(): Worker("test",0), m_io() {};
	~TestHost() { m_io.stop(); stopWorking(); }
	void start() { startWorking(); }
	void doWork() { m_io.run(); }
	
protected:
	ba::io_service m_io;
};

/**
 * Only used for testing. Not useful beyond tests.
 */
class TestNodeHost: public TestHost
{
public:
	TestNodeHost(): m_nodes(m_io) {};
	~TestNodeHost() { m_io.stop(); stopWorking(); }
	void start() { startWorking(); }
	void doWork() { m_io.run(); }

	NodeTable m_nodes;
};

class TestUDPSocket: UDPSocketEvents, public TestHost
{
public:
	TestUDPSocket(): m_socket(new UDPSocket<TestUDPSocket, 1024>(m_io, *this, 30300)) {}
	~TestUDPSocket() { m_io.stop(); stopWorking(); }
	void start() { startWorking(); }
	void doWork() { m_io.run(); }
	
	void onDisconnected(UDPSocketFace*) {};
	void onReceived(UDPSocketFace*, bi::udp::endpoint const&, bytesConstRef _packet) { if (_packet.toString() == "AAAA") success = true; }

	shared_ptr<UDPSocket<TestUDPSocket, 1024>> m_socket;
	
	bool success = false;
};

BOOST_AUTO_TEST_SUITE(p2p)

BOOST_AUTO_TEST_CASE(kademlia)
{
	TestNodeHost nodeHost;
	
}

BOOST_AUTO_TEST_CASE(test_txrx_one)
{
	UDPDatagram d(bi::udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 30300), bytes({65,65,65,65}));
	TestUDPSocket a; a.m_socket->connect(); a.start();
	a.m_socket->send(d);
	sleep(1);
	BOOST_REQUIRE_EQUAL(true, a.success);
}

BOOST_AUTO_TEST_SUITE_END()

