// SPDX-License-Identifier: GPL-3.0
/**
 * Translates Yul code from EVM dialect to Ewasm dialect.
 */

#pragma once

#include <libyul/AsmDataForward.h>
#include <libyul/optimiser/ASTWalker.h>
#include <libyul/Dialect.h>

namespace solidity::yul
{
struct Object;

class EVMToEwasmTranslator: public ASTModifier
{
public:
	EVMToEwasmTranslator(Dialect const& _evmDialect): m_dialect(_evmDialect) {}
	Object run(Object const& _object);

private:
	void parsePolyfill();

	Dialect const& m_dialect;

	std::shared_ptr<Block> m_polyfill;
	std::set<YulString> m_polyfillFunctions;
};

}
