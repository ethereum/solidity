// SPDX-License-Identifier: GPL-3.0
/**
 * Component that computes information relevant during decoding an external function
 * call's return values.
 */
#pragma once

#include <liblangutil/EVMVersion.h>
#include <libsolidity/ast/Types.h>

namespace solidity::frontend
{

/**
 * Computes and holds information relevant during decoding an external function
 * call's return values.
 */
struct ReturnInfo
{
	ReturnInfo(langutil::EVMVersion const& _evmVersion, FunctionType const& _functionType);

	/// Vector of TypePointer, for each return variable. Dynamic types are already replaced if required.
	TypePointers returnTypes = {};

	/// Boolean, indicating whether or not return size is only known at runtime.
	bool dynamicReturnSize = false;

	/// Contains the at compile time estimated return size.
	unsigned estimatedReturnSize = 0;
};

}
