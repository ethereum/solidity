/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * Yul dialects for EVM.
 */

#pragma once

#include <libyul/Dialect.h>

#include <libyul/backends/evm/AbstractAssembly.h>
#include <liblangutil/EVMVersion.h>

#include <map>

namespace yul
{

class YulString;
using Type = YulString;
struct FunctionCall;
struct Object;

/**
 * Context used during code generation.
 */
struct BuiltinContext
{
	Object const* currentObject = nullptr;
	/// Mapping from named objects to abstract assembly sub IDs.
	std::map<YulString, AbstractAssembly::SubID> subIDs;
};

struct BuiltinFunctionForEVM: BuiltinFunction
{
	/// Function to generate code for the given function call and append it to the abstract
	/// assembly. The fourth parameter is called to visit (and generate code for) the arguments
	/// from right to left.
	std::function<void(FunctionCall const&, AbstractAssembly&, BuiltinContext&, std::function<void()>)> generateCode;
};


/**
 * Yul dialect for EVM as a backend.
 * The main difference is that the builtin functions take an AbstractAssembly for the
 * code generation.
 */
struct EVMDialect: public Dialect
{
	EVMDialect(AsmFlavour _flavour, bool _objectAccess, langutil::EVMVersion _evmVersion);

	/// @returns the builtin function of the given name or a nullptr if it is not a builtin function.
	BuiltinFunctionForEVM const* builtin(YulString _name) const override;

	static std::shared_ptr<EVMDialect> looseAssemblyForEVM(langutil::EVMVersion _version);
	static std::shared_ptr<EVMDialect> strictAssemblyForEVM(langutil::EVMVersion _version);
	static std::shared_ptr<EVMDialect> strictAssemblyForEVMObjects(langutil::EVMVersion _version);
	static std::shared_ptr<EVMDialect> yulForEVM(langutil::EVMVersion _version);

	langutil::EVMVersion evmVersion() const { return m_evmVersion; }

	bool providesObjectAccess() const { return m_objectAccess; }

protected:
	void addFunction(
		std::string _name,
		size_t _params,
		size_t _returns,
		bool _movable,
		bool _sideEffectFree,
		bool _literalArguments,
		std::function<void(FunctionCall const&, AbstractAssembly&, BuiltinContext&, std::function<void()>)> _generateCode
	);

	bool m_objectAccess;
	langutil::EVMVersion m_evmVersion;
	std::map<YulString, BuiltinFunctionForEVM> m_functions;
};

}
