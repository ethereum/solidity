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
/** @file Common.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 *
 * Ethereum-specific data structures & algorithms.
 */

#pragma once

#include <string>
#include <functional>
#include <libdevcore/Common.h>
#include <libdevcore/FixedHash.h>
#include <libdevcrypto/Common.h>

namespace dev
{
namespace eth
{

/// Current protocol version.
extern const unsigned c_protocolVersion;

/// Current minor protocol version.
extern const unsigned c_minorProtocolVersion;

/// Current database version.
extern const unsigned c_databaseVersion;

/// User-friendly string representation of the amount _b in wei.
std::string formatBalance(bigint const& _b);

DEV_SIMPLE_EXCEPTION(InvalidAddress);

/// Convert the given string into an address.
Address toAddress(std::string const& _s);

/// Get information concerning the currency denominations.
std::vector<std::pair<u256, std::string>> const& units();

/// The log bloom's size (2048-bit).
using LogBloom = h2048;

/// Many log blooms.
using LogBlooms = std::vector<LogBloom>;

// The various denominations; here for ease of use where needed within code.
static const u256 ether = exp10<18>();
static const u256 finney = exp10<15>();
static const u256 szabo = exp10<12>();
static const u256 shannon = exp10<9>();
static const u256 wei = exp10<0>();

using Nonce = h64;

using BlockNumber = unsigned;

static const BlockNumber LatestBlock = (BlockNumber)-2;
static const BlockNumber PendingBlock = (BlockNumber)-1;
static const h256 LatestBlockHash = h256(2);
static const h256 EarliestBlockHash = h256(1);
static const h256 PendingBlockHash = h256(0);

static const u256 DefaultBlockGasLimit = 4712388;

enum class RelativeBlock: BlockNumber
{
	Latest = LatestBlock,
	Pending = PendingBlock
};

class Transaction;

struct ImportRoute
{
	h256s deadBlocks;
	h256s liveBlocks;
	std::vector<Transaction> goodTranactions;
};

enum class ImportResult
{
	Success = 0,
	UnknownParent,
	FutureTimeKnown,
	FutureTimeUnknown,
	AlreadyInChain,
	AlreadyKnown,
	Malformed,
	OverbidGasPrice,
	BadChain
};

struct ImportRequirements
{
	using value = unsigned;
	enum
	{
		ValidSeal = 1, ///< Validate seal
		UncleBasic = 4, ///< Check the basic structure of the uncles.
		TransactionBasic = 8, ///< Check the basic structure of the transactions.
		UncleSeals = 16, ///< Check the basic structure of the uncles.
		TransactionSignatures = 32, ///< Check the basic structure of the transactions.
		Parent = 64, ///< Check parent block header.
		UncleParent = 128, ///< Check uncle parent block header.
		PostGenesis = 256, ///< Require block to be non-genesis.
		CheckUncles = UncleBasic | UncleSeals, ///< Check uncle seals.
		CheckTransactions = TransactionBasic | TransactionSignatures, ///< Check transaction signatures.
		OutOfOrderChecks = ValidSeal | CheckUncles | CheckTransactions, ///< Do all checks that can be done independently of prior blocks having been imported.
		InOrderChecks = Parent | UncleParent, ///< Do all checks that cannot be done independently of prior blocks having been imported.
		Everything = ValidSeal | CheckUncles | CheckTransactions | Parent | UncleParent,
		None = 0
	};
};

/// Super-duper signal mechanism. TODO: replace with somthing a bit heavier weight.
template<typename... Args> class Signal
{
public:
	using Callback = std::function<void(Args...)>;

	class HandlerAux
	{
		friend class Signal;

	public:
		~HandlerAux() { if (m_s) m_s->m_fire.erase(m_i); }
		void reset() { m_s = nullptr; }
		void fire(Args const&... _args) { m_h(_args...); }

	private:
		HandlerAux(unsigned _i, Signal* _s, Callback const& _h): m_i(_i), m_s(_s), m_h(_h) {}

		unsigned m_i = 0;
		Signal* m_s = nullptr;
		Callback m_h;
	};

	~Signal()
	{
		for (auto const& h : m_fire)
			if (auto l = h.second.lock())
				l->reset();
	}

	std::shared_ptr<HandlerAux> add(Callback const& _h)
	{
		auto n = m_fire.empty() ? 0 : (m_fire.rbegin()->first + 1);
		auto h =  std::shared_ptr<HandlerAux>(new HandlerAux(n, this, _h));
		m_fire[n] = h;
		return h;
	}

	void operator()(Args const&... _args)
	{
		for (auto const& f: valuesOf(m_fire))
			if (auto h = f.lock())
				h->fire(_args...);
	}

private:
	std::map<unsigned, std::weak_ptr<typename Signal::HandlerAux>> m_fire;
};

template<class... Args> using Handler = std::shared_ptr<typename Signal<Args...>::HandlerAux>;

struct TransactionSkeleton
{
	bool creation = false;
	Address from;
	Address to;
	u256 value;
	bytes data;
	u256 nonce = Invalid256;
	u256 gas = Invalid256;
	u256 gasPrice = Invalid256;

	std::string userReadable(bool _toProxy, std::function<std::pair<bool, std::string>(TransactionSkeleton const&)> const& _getNatSpec, std::function<std::string(Address const&)> const& _formatAddress) const;
};


void badBlock(bytesConstRef _header, std::string const& _err);
inline void badBlock(bytes const& _header, std::string const& _err) { badBlock(&_header, _err); }

// TODO: move back into a mining subsystem and have it be accessible from Sealant only via a dynamic_cast.
/**
 * @brief Describes the progress of a mining operation.
 */
struct WorkingProgress
{
//	MiningProgress& operator+=(MiningProgress const& _mp) { hashes += _mp.hashes; ms = std::max(ms, _mp.ms); return *this; }
	uint64_t hashes = 0;		///< Total number of hashes computed.
	uint64_t ms = 0;			///< Total number of milliseconds of mining thus far.
	u256 rate() const { return ms == 0 ? 0 : hashes * 1000 / ms; }
};

/// Import transaction policy
enum class IfDropped
{
	Ignore, ///< Don't import transaction that was previously dropped.
	Retry 	///< Import transaction even if it was dropped before.
};

}
}
