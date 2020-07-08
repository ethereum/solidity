// SPDX-License-Identifier: GPL-3.0
/**
 * Information generated during analyzer part of inline assembly.
 */

#pragma once

#include <libyul/AsmDataForward.h>

#include <map>
#include <memory>
#include <vector>

namespace solidity::yul
{

struct Scope;

struct AsmAnalysisInfo
{
	using StackHeightInfo = std::map<void const*, int>;
	using Scopes = std::map<Block const*, std::shared_ptr<Scope>>;
	Scopes scopes;
	/// Virtual blocks which will be used for scopes for function arguments and return values.
	std::map<FunctionDefinition const*, std::shared_ptr<Block const>> virtualBlocks;
};

}
