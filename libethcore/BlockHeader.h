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
/** @file BlockHeader.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <algorithm>
#include <libdevcore/Common.h>
#include <libdevcore/RLP.h>
#include <libdevcore/SHA3.h>
#include "Common.h"
#include "ChainOperationParams.h"
#include "Exceptions.h"

namespace dev
{
namespace eth
{

enum IncludeSeal
{
	WithoutSeal = 0,
	WithSeal = 1,
	OnlySeal = 2
};

enum Strictness
{
	CheckEverything,
	JustSeal,
	QuickNonce,
	IgnoreSeal,
	CheckNothingNew
};

// TODO: for implementing soon.
/*enum Check
{
	CheckBasic,
	CheckExtended,
	CheckBlock,
	CheckParent,
	CheckSeal,
	CheckSealQuickly,
	CheckAll = CheckBasic | CheckExtended | CheckBlock | CheckParent | CheckSeal,
};
using Checks = FlagSet<Check>;*/

enum BlockDataType
{
	HeaderData,
	BlockData
};

DEV_SIMPLE_EXCEPTION(NoHashRecorded);
DEV_SIMPLE_EXCEPTION(GenesisBlockCannotBeCalculated);

/** @brief Encapsulation of a block header.
 * Class to contain all of a block header's data. It is able to parse a block header and populate
 * from some given RLP block serialisation with the static fromHeader(), through the method
 * populate(). This will not conduct any verification above basic formating. In this case extra
 * verification can be performed through verify().
 *
 * The object may also be populated from an entire block through the explicit
 * constructor BlockHeader(bytesConstRef) and manually with the populate() method. These will
 * conduct verification of the header against the other information in the block.
 *
 * The object may be populated with a template given a parent BlockHeader object with the
 * populateFromParent() method. The genesis block info may be retrieved with genesis() and the
 * corresponding RLP block created with createGenesisBlock().
 *
 * To determine the header hash without the nonce (for sealing), the method hash(WithoutNonce) is
 * provided.
 *
 * The default constructor creates an empty object, which can be tested against with the boolean
 * conversion operator.
 */
class BlockHeader
{
	friend class BlockChain;
public:
	static const unsigned BasicFields = 13;

	BlockHeader();
	explicit BlockHeader(bytesConstRef _data, BlockDataType _bdt = BlockData, h256 const& _hashWith = h256());
	explicit BlockHeader(bytes const& _data, BlockDataType _bdt = BlockData, h256 const& _hashWith = h256()): BlockHeader(&_data, _bdt, _hashWith) {}

	static h256 headerHashFromBlock(bytes const& _block) { return headerHashFromBlock(&_block); }
	static h256 headerHashFromBlock(bytesConstRef _block);
	static RLP extractHeader(bytesConstRef _block);

	explicit operator bool() const { return m_timestamp != Invalid256; }

	bool operator==(BlockHeader const& _cmp) const
	{
		return m_parentHash == _cmp.parentHash() &&
			m_sha3Uncles == _cmp.sha3Uncles() &&
			m_author == _cmp.author() &&
			m_stateRoot == _cmp.stateRoot() &&
			m_transactionsRoot == _cmp.transactionsRoot() &&
			m_receiptsRoot == _cmp.receiptsRoot() &&
			m_logBloom == _cmp.logBloom() &&
			m_difficulty == _cmp.difficulty() &&
			m_number == _cmp.number() &&
			m_gasLimit == _cmp.gasLimit() &&
			m_gasUsed == _cmp.gasUsed() &&
			m_timestamp == _cmp.timestamp() &&
			m_extraData == _cmp.extraData();
	}
	bool operator!=(BlockHeader const& _cmp) const { return !operator==(_cmp); }

	void clear();
	void noteDirty() const { m_hashWithout = m_hash = h256(); }
	void populateFromParent(BlockHeader const& parent);

	// TODO: pull out into abstract class Verifier.
	void verify(Strictness _s = CheckEverything, BlockHeader const& _parent = BlockHeader(), bytesConstRef _block = bytesConstRef()) const;
	void verify(Strictness _s, bytesConstRef _block) const { verify(_s, BlockHeader(), _block); }

	h256 hash(IncludeSeal _i = WithSeal) const;
	void streamRLP(RLPStream& _s, IncludeSeal _i = WithSeal) const;

	void setParentHash(h256 const& _v) { m_parentHash = _v; noteDirty(); }
	void setSha3Uncles(h256 const& _v) { m_sha3Uncles = _v; noteDirty(); }
	void setTimestamp(u256 const& _v) { m_timestamp = _v; noteDirty(); }
	void setAuthor(Address const& _v) { m_author = _v; noteDirty(); }
	void setRoots(h256 const& _t, h256 const& _r, h256 const& _u, h256 const& _s) { m_transactionsRoot = _t; m_receiptsRoot = _r; m_stateRoot = _s; m_sha3Uncles = _u; noteDirty(); }
	void setGasUsed(u256 const& _v) { m_gasUsed = _v; noteDirty(); }
	void setNumber(u256 const& _v) { m_number = _v; noteDirty(); }
	void setGasLimit(u256 const& _v) { m_gasLimit = _v; noteDirty(); }
	void setExtraData(bytes const& _v) { m_extraData = _v; noteDirty(); }
	void setLogBloom(LogBloom const& _v) { m_logBloom = _v; noteDirty(); }
	void setDifficulty(u256 const& _v) { m_difficulty = _v; noteDirty(); }
	template <class T> void setSeal(unsigned _offset, T const& _value) { if (m_seal.size() <= _offset) m_seal.resize(_offset + 1); m_seal[_offset] = rlp(_value); noteDirty(); }
	template <class T> void setSeal(T const& _value) { setSeal(0, _value); }

	h256 const& parentHash() const { return m_parentHash; }
	h256 const& sha3Uncles() const { return m_sha3Uncles; }
	u256 const& timestamp() const { return m_timestamp; }
	Address const& author() const { return m_author; }
	h256 const& stateRoot() const { return m_stateRoot; }
	h256 const& transactionsRoot() const { return m_transactionsRoot; }
	h256 const& receiptsRoot() const { return m_receiptsRoot; }
	u256 const& gasUsed() const { return m_gasUsed; }
	u256 const& number() const { return m_number; }
	u256 const& gasLimit() const { return m_gasLimit; }
	bytes const& extraData() const { return m_extraData; }
	LogBloom const& logBloom() const { return m_logBloom; }
	u256 const& difficulty() const { return m_difficulty; }
	template <class T> T seal(unsigned _offset = 0) const { T ret; if (_offset < m_seal.size()) ret = RLP(m_seal[_offset]).convert<T>(RLP::VeryStrict); return ret; }

private:
	void populate(RLP const& _header);
	void streamRLPFields(RLPStream& _s) const;

	h256 m_parentHash;
	h256 m_sha3Uncles;
	h256 m_stateRoot;
	h256 m_transactionsRoot;
	h256 m_receiptsRoot;
	LogBloom m_logBloom;
	u256 m_number;
	u256 m_gasLimit;
	u256 m_gasUsed;
	bytes m_extraData;
	u256 m_timestamp = Invalid256;

	Address m_author;
	u256 m_difficulty;

	std::vector<bytes> m_seal;		///< Additional (RLP-encoded) header fields.

	mutable h256 m_hash;			///< (Memoised) SHA3 hash of the block header with seal.
	mutable h256 m_hashWithout;		///< (Memoised) SHA3 hash of the block header without seal.
};

inline std::ostream& operator<<(std::ostream& _out, BlockHeader const& _bi)
{
	_out << _bi.hash(WithoutSeal) << " " << _bi.parentHash() << " " << _bi.sha3Uncles() << " " << _bi.author() << " " << _bi.stateRoot() << " " << _bi.transactionsRoot() << " " <<
			_bi.receiptsRoot() << " " << _bi.logBloom() << " " << _bi.difficulty() << " " << _bi.number() << " " << _bi.gasLimit() << " " <<
			_bi.gasUsed() << " " << _bi.timestamp();
	return _out;
}

}
}
