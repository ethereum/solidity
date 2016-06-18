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
/** @file ExtVMFace.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <set>
#include <functional>
#include <libdevcore/Common.h>
#include <libdevcore/CommonData.h>
#include <libdevcore/RLP.h>
#include <libdevcore/SHA3.h>
#include <libevmcore/Instruction.h>
#include <libethcore/Common.h>
#include <libethcore/BlockHeader.h>
#include <libethcore/ChainOperationParams.h>

namespace dev
{
namespace eth
{

enum class BlockPolarity
{
	Unknown,
	Dead,
	Live
};

struct LogEntry
{
	LogEntry() {}
	LogEntry(RLP const& _r) { address = (Address)_r[0]; topics = _r[1].toVector<h256>(); data = _r[2].toBytes(); }
	LogEntry(Address const& _address, h256s const& _ts, bytes&& _d): address(_address), topics(_ts), data(std::move(_d)) {}

	void streamRLP(RLPStream& _s) const { _s.appendList(3) << address << topics << data; }

	LogBloom bloom() const
	{
		LogBloom ret;
		ret.shiftBloom<3>(sha3(address.ref()));
		for (auto t: topics)
			ret.shiftBloom<3>(sha3(t.ref()));
		return ret;
	}

	Address address;
	h256s topics;
	bytes data;
};

using LogEntries = std::vector<LogEntry>;

struct LocalisedLogEntry: public LogEntry
{
	LocalisedLogEntry() {}
	explicit LocalisedLogEntry(LogEntry const& _le): LogEntry(_le) {}

	explicit LocalisedLogEntry(
		LogEntry const& _le,
		h256 _special
	):
		LogEntry(_le),
		isSpecial(true),
		special(_special)
	{}

	explicit LocalisedLogEntry(
		LogEntry const& _le,
		h256 const& _blockHash,
		BlockNumber _blockNumber,
		h256 const& _transactionHash,
		unsigned _transactionIndex,
		unsigned _logIndex,
		BlockPolarity _polarity = BlockPolarity::Unknown
	):
		LogEntry(_le),
		blockHash(_blockHash),
		blockNumber(_blockNumber),
		transactionHash(_transactionHash),
		transactionIndex(_transactionIndex),
		logIndex(_logIndex),
		polarity(_polarity),
		mined(true)
	{}

	h256 blockHash;
	BlockNumber blockNumber = 0;
	h256 transactionHash;
	unsigned transactionIndex = 0;
	unsigned logIndex = 0;
	BlockPolarity polarity = BlockPolarity::Unknown;
	bool mined = false;
	bool isSpecial = false;
	h256 special;
};

using LocalisedLogEntries = std::vector<LocalisedLogEntry>;

inline LogBloom bloom(LogEntries const& _logs)
{
	LogBloom ret;
	for (auto const& l: _logs)
		ret |= l.bloom();
	return ret;
}

struct SubState
{
	std::set<Address> suicides;	///< Any accounts that have suicided.
	LogEntries logs;			///< Any logs.
	u256 refunds;				///< Refund counter of SSTORE nonzero->zero.

	SubState& operator+=(SubState const& _s)
	{
		suicides += _s.suicides;
		refunds += _s.refunds;
		logs += _s.logs;
		return *this;
	}

	void clear()
	{
		suicides.clear();
		logs.clear();
		refunds = 0;
	}
};

class ExtVMFace;
class VM;

using LastHashes = std::vector<h256>;

using OnOpFunc = std::function<void(uint64_t /*steps*/, Instruction /*instr*/, bigint /*newMemSize*/, bigint /*gasCost*/, bigint /*gas*/, VM*, ExtVMFace const*)>;

struct CallParameters
{
	Address senderAddress;
	Address codeAddress;
	Address receiveAddress;
	u256 valueTransfer;
	u256 apparentValue;
	u256 gas;
	bytesConstRef data;
	bytesRef out;
	OnOpFunc onOp;
};

class EnvInfo
{
public:
	EnvInfo() {}
	EnvInfo(BlockHeader const& _current, LastHashes const& _lh = LastHashes(), u256 const& _gasUsed = u256()):
		m_number(_current.number()),
		m_author(_current.author()),
		m_timestamp(_current.timestamp()),
		m_difficulty(_current.difficulty()),
		m_gasLimit(_current.gasLimit()),
		m_lastHashes(_lh),
		m_gasUsed(_gasUsed)
	{}

	EnvInfo(BlockHeader const& _current, LastHashes&& _lh, u256 const& _gasUsed = u256()):
		m_number(_current.number()),
		m_author(_current.author()),
		m_timestamp(_current.timestamp()),
		m_difficulty(_current.difficulty()),
		m_gasLimit(_current.gasLimit()),
		m_lastHashes(_lh),
		m_gasUsed(_gasUsed)
	{}

	u256 const& number() const { return m_number; }
	Address const& author() const { return m_author; }
	u256 const& timestamp() const { return m_timestamp; }
	u256 const& difficulty() const { return m_difficulty; }
	u256 const& gasLimit() const { return m_gasLimit; }
	LastHashes const& lastHashes() const { return m_lastHashes; }
	u256 const& gasUsed() const { return m_gasUsed; }

	void setNumber(u256 const& _v) { m_number = _v; }
	void setAuthor(Address const& _v) { m_author = _v; }
	void setTimestamp(u256 const& _v) { m_timestamp = _v; }
	void setDifficulty(u256 const& _v) { m_difficulty = _v; }
	void setGasLimit(u256 const& _v) { m_gasLimit = _v; }
	void setLastHashes(LastHashes const& _lh) { m_lastHashes = _lh; }
	void setLastHashes(LastHashes&& _lh) { m_lastHashes = _lh; }
	void setGasUsed(u256 const& _v) { m_gasUsed = _v; }

private:
	u256 m_number;
	Address m_author;
	u256 m_timestamp;
	u256 m_difficulty;
	u256 m_gasLimit;
	LastHashes m_lastHashes;
	u256 m_gasUsed;
};

/**
 * @brief Interface and null implementation of the class for specifying VM externalities.
 */
class ExtVMFace
{
public:
	/// Null constructor.
	ExtVMFace() = default;

	/// Full constructor.
	ExtVMFace(EnvInfo const& _envInfo, Address _myAddress, Address _caller, Address _origin, u256 _value, u256 _gasPrice, bytesConstRef _data, bytes _code, h256 const& _codeHash, unsigned _depth);

	virtual ~ExtVMFace() = default;

	ExtVMFace(ExtVMFace const&) = delete;
	void operator=(ExtVMFace) = delete;

	/// Read storage location.
	virtual u256 store(u256) { return 0; }

	/// Write a value in storage.
	virtual void setStore(u256, u256) {}

	/// Read address's balance.
	virtual u256 balance(Address) { return 0; }

	/// Read address's code.
	virtual bytes const& codeAt(Address) { return NullBytes; }

	/// Subtract amount from account's balance.
	virtual void subBalance(u256) {}

	/// Determine account's TX count.
	virtual u256 txCount(Address) { return 0; }

	/// Does the account exist?
	virtual bool exists(Address) { return false; }

	/// Suicide the associated contract and give proceeds to the given address.
	virtual void suicide(Address) { sub.suicides.insert(myAddress); }

	/// Create a new (contract) account.
	virtual h160 create(u256, u256&, bytesConstRef, OnOpFunc const&) { return h160(); }

	/// Make a new message call.
	virtual bool call(CallParameters&) { return false; }

	/// Revert any changes made (by any of the other calls).
	virtual void log(h256s&& _topics, bytesConstRef _data) { sub.logs.push_back(LogEntry(myAddress, std::move(_topics), _data.toBytes())); }

	/// Revert any changes made (by any of the other calls).
	virtual void revert() {}

	/// Hash of a block if within the last 256 blocks, or h256() otherwise.
	h256 blockHash(u256 _number) { return _number < envInfo().number() && _number >= (std::max<u256>(256, envInfo().number()) - 256) ? envInfo().lastHashes()[(unsigned)(envInfo().number() - 1 - _number)] : h256(); }

	/// Get the code at the given location in code ROM.
	byte getCode(u256 _n) const { return _n < code.size() ? code[(size_t)_n] : 0; }

	/// Get the execution environment information.
	EnvInfo const& envInfo() const { return m_envInfo; }

	/// Return the EVM gas-price schedule for this execution context.
	virtual EVMSchedule const& evmSchedule() const { return DefaultSchedule; }

private:
	EnvInfo const& m_envInfo;

public:
	// TODO: make private
	Address myAddress;			///< Address associated with executing code (a contract, or contract-to-be).
	Address caller;				///< Address which sent the message (either equal to origin or a contract).
	Address origin;				///< Original transactor.
	u256 value;					///< Value (in Wei) that was passed to this address.
	u256 gasPrice;				///< Price of gas (that we already paid).
	bytesConstRef data;			///< Current input data.
	bytes code;					///< Current code that is executing.
	h256 codeHash;				///< SHA3 hash of the executing code
	SubState sub;				///< Sub-band VM state (suicides, refund counter, logs).
	unsigned depth = 0;			///< Depth of the present call.
};

}
}
