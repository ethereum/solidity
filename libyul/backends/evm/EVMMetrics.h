// SPDX-License-Identifier: GPL-3.0
/**
 * Module providing metrics for the optimizer.
 */

#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <liblangutil/EVMVersion.h>
#include <libevmasm/Instruction.h>

namespace solidity::yul
{

struct EVMDialect;

/**
 * Gas meter for expressions only involving literals, identifiers and
 * EVM instructions.
 *
 * Assumes that EXP is not used with exponents larger than a single byte.
 * Is not particularly exact for anything apart from arithmetic.
 */
class GasMeter
{
public:
	GasMeter(EVMDialect const& _dialect, bool _isCreation, size_t _runs):
		m_dialect(_dialect),
		m_isCreation{_isCreation},
		m_runs(_runs)
	{}

	/// @returns the full combined costs of deploying and evaluating the expression.
	size_t costs(Expression const& _expression) const;
	/// @returns the combined costs of deploying and running the instruction, not including
	/// the costs for its arguments.
	size_t instructionCosts(evmasm::Instruction _instruction) const;

private:
	size_t combineCosts(std::pair<size_t, size_t> _costs) const;

	EVMDialect const& m_dialect;
	bool m_isCreation = false;
	size_t m_runs;
};

class GasMeterVisitor: public ASTWalker
{
public:
	static std::pair<size_t, size_t> costs(
		Expression const& _expression,
		EVMDialect const& _dialect,
		bool _isCreation
	);

	static std::pair<size_t, size_t> instructionCosts(
		evmasm::Instruction _instruction,
		EVMDialect const& _dialect,
		bool _isCreation = false
	);

public:
	GasMeterVisitor(EVMDialect const& _dialect, bool _isCreation):
		m_dialect(_dialect),
		m_isCreation{_isCreation}
	{}

	void operator()(FunctionCall const& _funCall) override;
	void operator()(Literal const& _literal) override;
	void operator()(Identifier const& _identifier) override;

private:
	size_t singleByteDataGas() const;
	/// Computes the cost of storing and executing the single instruction (excluding its arguments).
	/// For EXP, it assumes that the exponent is at most 255.
	/// Does not work particularly exact for anything apart from arithmetic.
	void instructionCostsInternal(evmasm::Instruction _instruction);

	EVMDialect const& m_dialect;
	bool m_isCreation = false;
	size_t m_runGas = 0;
	size_t m_dataGas = 0;
};

}
