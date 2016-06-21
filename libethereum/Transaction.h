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
/** @file Transaction.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <libdevcore/RLP.h>
#include <libdevcore/SHA3.h>
#include <libethcore/Common.h>
#include <libethcore/Transaction.h>
#include <libethcore/ChainOperationParams.h>

namespace dev
{
namespace eth
{

enum class TransactionException
{
	None = 0,
	Unknown,
	BadRLP,
	InvalidFormat,
	OutOfGasIntrinsic,		///< Too little gas to pay for the base transaction cost.
	InvalidSignature,
	InvalidNonce,
	NotEnoughCash,
	OutOfGasBase,			///< Too little gas to pay for the base transaction cost.
	BlockGasLimitReached,
	BadInstruction,
	BadJumpDestination,
	OutOfGas,				///< Ran out of gas executing code of the transaction.
	OutOfStack,				///< Ran out of stack executing code of the transaction.
	StackUnderflow
};

enum class CodeDeposit
{
	None = 0,
	Failed,
	Success
};

struct VMException;

TransactionException toTransactionException(Exception const& _e);
std::ostream& operator<<(std::ostream& _out, TransactionException const& _er);

/// Description of the result of executing a transaction.
struct ExecutionResult
{
	u256 gasUsed = 0;
	TransactionException excepted = TransactionException::Unknown;
	Address newAddress;
	bytes output;
	CodeDeposit codeDeposit = CodeDeposit::None;					///< Failed if an attempted deposit failed due to lack of gas.
	u256 gasRefunded = 0;
	unsigned depositSize = 0; 										///< Amount of code of the creation's attempted deposit.
	u256 gasForDeposit; 											///< Amount of gas remaining for the code deposit phase.
};

std::ostream& operator<<(std::ostream& _out, ExecutionResult const& _er);

/// Encodes a transaction, ready to be exported to or freshly imported from RLP.
class Transaction: public TransactionBase
{
public:
	/// Constructs a null transaction.
	Transaction() {}

	/// Constructs from a transaction skeleton & optional secret.
	Transaction(TransactionSkeleton const& _ts, Secret const& _s = Secret()): TransactionBase(_ts, _s) {}

	/// Constructs a signed message-call transaction.
	Transaction(u256 const& _value, u256 const& _gasPrice, u256 const& _gas, Address const& _dest, bytes const& _data, u256 const& _nonce, Secret const& _secret):
		TransactionBase(_value, _gasPrice, _gas, _dest, _data, _nonce, _secret)
	{}

	/// Constructs a signed contract-creation transaction.
	Transaction(u256 const& _value, u256 const& _gasPrice, u256 const& _gas, bytes const& _data, u256 const& _nonce, Secret const& _secret):
		TransactionBase(_value, _gasPrice, _gas, _data, _nonce, _secret)
	{}

	/// Constructs an unsigned message-call transaction.
	Transaction(u256 const& _value, u256 const& _gasPrice, u256 const& _gas, Address const& _dest, bytes const& _data, u256 const& _nonce = Invalid256):
		TransactionBase(_value, _gasPrice, _gas, _dest, _data, _nonce)
	{}

	/// Constructs an unsigned contract-creation transaction.
	Transaction(u256 const& _value, u256 const& _gasPrice, u256 const& _gas, bytes const& _data, u256 const& _nonce = Invalid256):
		TransactionBase(_value, _gasPrice, _gas, _data, _nonce)
	{}

	/// Constructs a transaction from the given RLP.
	explicit Transaction(bytesConstRef _rlp, CheckTransaction _checkSig);

	/// Constructs a transaction from the given RLP.
	explicit Transaction(bytes const& _rlp, CheckTransaction _checkSig): Transaction(&_rlp, _checkSig) {}
};

/// Nice name for vector of Transaction.
using Transactions = std::vector<Transaction>;

class LocalisedTransaction: public Transaction
{
public:
	LocalisedTransaction(
		Transaction const& _t,
		h256 const& _blockHash,
		unsigned _transactionIndex,
		BlockNumber _blockNumber = 0
	):
		Transaction(_t),
		m_blockHash(_blockHash),
		m_transactionIndex(_transactionIndex),
		m_blockNumber(_blockNumber)
	{}

	h256 const& blockHash() const { return m_blockHash; }
	unsigned transactionIndex() const { return m_transactionIndex; }
	BlockNumber blockNumber() const { return m_blockNumber; }

private:
	h256 m_blockHash;
	unsigned m_transactionIndex;
	BlockNumber m_blockNumber;
};

}
}
