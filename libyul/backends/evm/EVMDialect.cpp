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

#include <libyul/backends/evm/EVMDialect.h>

#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AsmData.h>
#include <libyul/Object.h>
#include <libyul/backends/evm/AbstractAssembly.h>

#include <liblangutil/Exceptions.h>

#include <libyul/Exceptions.h>

#include <boost/range/adaptor/reversed.hpp>

using namespace std;
using namespace dev;
using namespace yul;

namespace
{
pair<YulString, BuiltinFunctionForEVM> createFunction(
	string _name,
	size_t _params,
	size_t _returns,
	bool _movable,
	bool _sideEffectFree,
	bool _literalArguments,
	std::function<void(FunctionCall const&, AbstractAssembly&, BuiltinContext&, std::function<void()>)> _generateCode
)
{
	YulString name{std::move(_name)};
	BuiltinFunctionForEVM f;
	f.name = name;
	f.parameters.resize(_params);
	f.returns.resize(_returns);
	f.movable = _movable;
	f.literalArguments = _literalArguments;
	f.sideEffectFree = _sideEffectFree;
	f.generateCode = std::move(_generateCode);
	return {name, f};
}

map<YulString, BuiltinFunctionForEVM> createBuiltins(langutil::EVMVersion, bool _objectAccess)
{
	map<YulString, BuiltinFunctionForEVM> builtins;
	if (_objectAccess)
	{
		builtins.emplace(createFunction("datasize", 1, 1, true, true, true, [](
			FunctionCall const& _call,
			AbstractAssembly& _assembly,
			BuiltinContext& _context,
			function<void()>
		) {
			yulAssert(_context.currentObject, "No object available.");
			yulAssert(_call.arguments.size() == 1, "");
			Expression const& arg = _call.arguments.front();
			YulString dataName = boost::get<Literal>(arg).value;
			if (_context.currentObject->name == dataName)
				_assembly.appendAssemblySize();
			else
			{
				yulAssert(
					_context.subIDs.count(dataName) != 0,
					"Could not find assembly object <" + dataName.str() + ">."
				);
				_assembly.appendDataSize(_context.subIDs.at(dataName));
			}
		}));
		builtins.emplace(createFunction("dataoffset", 1, 1, true, true, true, [](
			FunctionCall const& _call,
			AbstractAssembly& _assembly,
			BuiltinContext& _context,
			std::function<void()>
		) {
			yulAssert(_context.currentObject, "No object available.");
			yulAssert(_call.arguments.size() == 1, "");
			Expression const& arg = _call.arguments.front();
			YulString dataName = boost::get<Literal>(arg).value;
			if (_context.currentObject->name == dataName)
				_assembly.appendConstant(0);
			else
			{
				yulAssert(
					_context.subIDs.count(dataName) != 0,
					"Could not find assembly object <" + dataName.str() + ">."
				);
				_assembly.appendDataOffset(_context.subIDs.at(dataName));
			}
		}));
		builtins.emplace(createFunction("datacopy", 3, 0, false, false, false, [](
			FunctionCall const&,
			AbstractAssembly& _assembly,
			BuiltinContext&,
			std::function<void()> _visitArguments
		) {
			_visitArguments();
			_assembly.appendInstruction(dev::eth::Instruction::CODECOPY);
		}));
	}
	return builtins;
}

}

EVMDialect::EVMDialect(AsmFlavour _flavour, bool _objectAccess, langutil::EVMVersion _evmVersion):
	Dialect{_flavour},
	m_objectAccess(_objectAccess),
	m_evmVersion(_evmVersion),
	m_functions(createBuiltins(_evmVersion, _objectAccess))
{
}

BuiltinFunctionForEVM const* EVMDialect::builtin(YulString _name) const
{
	auto it = m_functions.find(_name);
	if (it != m_functions.end())
		return &it->second;
	else
		return nullptr;
}

shared_ptr<EVMDialect const> EVMDialect::looseAssemblyForEVM(langutil::EVMVersion _version)
{
	return make_shared<EVMDialect>(AsmFlavour::Loose, false, _version);
}

shared_ptr<EVMDialect const> EVMDialect::strictAssemblyForEVM(langutil::EVMVersion _version)
{
	return make_shared<EVMDialect>(AsmFlavour::Strict, false, _version);
}

shared_ptr<EVMDialect const> EVMDialect::strictAssemblyForEVMObjects(langutil::EVMVersion _version)
{
	return make_shared<EVMDialect>(AsmFlavour::Strict, true, _version);
}

shared_ptr<EVMDialect const> EVMDialect::yulForEVM(langutil::EVMVersion _version)
{
	return make_shared<EVMDialect>(AsmFlavour::Yul, false, _version);
}
