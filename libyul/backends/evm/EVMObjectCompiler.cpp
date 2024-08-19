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
// SPDX-License-Identifier: GPL-3.0
/**
 * Compiler that transforms Yul Objects to EVM bytecode objects.
 */

#include <libyul/backends/evm/EVMObjectCompiler.h>

#include <libyul/backends/evm/EVMCodeTransform.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/backends/evm/OptimizedEVMCodeTransform.h>

#include <libyul/optimiser/FunctionCallFinder.h>

#include <libyul/Object.h>
#include <libyul/Exceptions.h>

#include <boost/algorithm/string.hpp>

using namespace solidity::yul;

void EVMObjectCompiler::compile(
	Object const& _object,
	AbstractAssembly& _assembly,
	EVMDialect const& _dialect,
	bool _optimize,
	std::optional<uint8_t> _eofVersion
)
{
	EVMObjectCompiler compiler(_assembly, _dialect, _eofVersion);
	compiler.run(_object, _optimize);
}

void EVMObjectCompiler::run(Object const& _object, bool _optimize)
{
	BuiltinContext context;
	context.currentObject = &_object;


	for (auto const& subNode: _object.subObjects)
		if (auto* subObject = dynamic_cast<Object*>(subNode.get()))
		{
			bool isCreation = !boost::ends_with(subObject->name, "_deployed");
			auto subAssemblyAndID = m_assembly.createSubAssembly(isCreation, subObject->name);
			context.subIDs[subObject->name] = subAssemblyAndID.second;
			subObject->subId = subAssemblyAndID.second;
			compile(*subObject, *subAssemblyAndID.first, m_dialect, _optimize, m_eofVersion);
		}
		else
		{
			Data const& data = dynamic_cast<Data const&>(*subNode);
			// Special handling of metadata.
			if (data.name == Object::metadataName())
				m_assembly.appendToAuxiliaryData(data.data);
			else
				context.subIDs[data.name] = m_assembly.appendData(data.data);
		}

	yulAssert(_object.analysisInfo, "No analysis info.");
	yulAssert(_object.hasCode(), "No code.");
	if (m_eofVersion.has_value())
		yulAssert(
			_optimize && (m_dialect.evmVersion() >= langutil::EVMVersion::prague()),
			"Experimental EOF support is only available for optimized via-IR compilation and the most recent EVM version."
		);
	if (_optimize && m_dialect.evmVersion().canOverchargeGasForCall())
	{
		auto stackErrors = OptimizedEVMCodeTransform::run(
			m_assembly,
			*_object.analysisInfo,
			_object.code()->root(),
			m_dialect,
			context,
			OptimizedEVMCodeTransform::UseNamedLabels::ForFirstFunctionOfEachName
		);
		if (!stackErrors.empty())
		{
			std::vector<FunctionCall const*> memoryGuardCalls = findFunctionCalls(
				_object.code()->root(),
				"memoryguard",
				m_dialect
			);
			auto stackError = stackErrors.front();
			std::string msg = stackError.comment() ? *stackError.comment() : "";
			if (memoryGuardCalls.empty())
				msg += "\nNo memoryguard was present. "
					"Consider using memory-safe assembly only and annotating it via "
					"'assembly (\"memory-safe\") { ... }'.";
			else
				msg += "\nmemoryguard was present.";
			stackError << util::errinfo_comment(msg);
			BOOST_THROW_EXCEPTION(stackError);
		}
	}
	else
	{
		// We do not catch and re-throw the stack too deep exception here because it is a YulException,
		// which should be native to this part of the code.
		CodeTransform transform{
			m_assembly,
			*_object.analysisInfo,
			_object.code()->root(),
			m_dialect,
			context,
			_optimize,
			{},
			CodeTransform::UseNamedLabels::ForFirstFunctionOfEachName
		};
		transform(_object.code()->root());
		if (!transform.stackErrors().empty())
			BOOST_THROW_EXCEPTION(transform.stackErrors().front());
	}
}
