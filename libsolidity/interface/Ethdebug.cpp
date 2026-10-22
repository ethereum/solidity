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

#include <libsolidity/interface/Ethdebug.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/Types.h>
#include <libsolidity/interface/EthdebugPointerTemplateRegistry.h>
#include <libsolidity/interface/EthdebugTypeRegistry.h>

#include <set>

using namespace solidity;
using namespace solidity::frontend;

void ethdebug::Resources::merge(Resources _other)
{
	for (auto& [id, document]: _other.types)
		types.insert_or_assign(id, std::move(document));
	for (auto& [name, pointerTemplate]: _other.pointers)
		pointers.insert_or_assign(name, std::move(pointerTemplate));
}

ethdebug::Resources ethdebug::resources(ContractDefinition const& _contract, std::map<std::string, unsigned> const& _sourceIndices)
{
	TypeRegistry types{_sourceIndices};

	auto const* typeType = dynamic_cast<TypeType const*>(_contract.type());
	solAssert(typeType, "Contract TypeType expected.");
	auto const* contractType = dynamic_cast<ContractType const*>(typeType->actualType());
	solAssert(contractType, "Contract type expected.");

	// The pointer templates describe the storage types of the state variables;
	// where each variable's value starts is the storage layout's to say.
	PointerTemplateRegistry templates;
	auto const addStateVariables = [&](DataLocation _dataLocation) {
		for (auto const& entry: contractType->linearizedStateVariables(_dataLocation))
		{
			VariableDeclaration const* variable = std::get<0>(entry);
			if (variable->name().empty())
				continue;
			types.registerType(*variable->annotation().type);
			templates.registerType(*variable->annotation().type);
		}
	};
	addStateVariables(DataLocation::Storage);
	addStateVariables(DataLocation::Transient);

	// Inherited functions and modifiers are compiled into the most derived
	// contract, so every linearized base contract contributes its types.
	for (ContractDefinition const* contract: _contract.annotation().linearizedBaseContracts)
	{
		for (FunctionDefinition const* function: contract->definedFunctions())
			types.registerCallableTypes(*function);
		for (ModifierDefinition const* modifier: contract->functionModifiers())
			types.registerCallableTypes(*modifier);
	}

	// Free functions and internal library functions reachable through imports
	// are compiled into the contract as well.
	SourceUnit const& sourceUnit = _contract.sourceUnit();
	std::set<SourceUnit const*> sourceUnits = sourceUnit.referencedSourceUnits(true);
	sourceUnits.insert(&sourceUnit);
	for (SourceUnit const* unit: sourceUnits)
		for (ASTPointer<ASTNode> const& node: unit->nodes())
		{
			if (auto const* freeFunction = dynamic_cast<FunctionDefinition const*>(node.get()))
				types.registerCallableTypes(*freeFunction);
			else if (auto const* library = dynamic_cast<ContractDefinition const*>(node.get()); library && library->isLibrary())
				for (FunctionDefinition const* function: library->definedFunctions())
					if (function->visibility() <= Visibility::Internal)
						types.registerCallableTypes(*function);
		}

	return Resources{
		.types = types.takeDocuments(),
		.pointers = templates.takeTemplates()
	};
}
