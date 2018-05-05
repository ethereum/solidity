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
 * @author Santiago Palladino <spalladino@gmail.com>
 * @date 2018
 * Outputs contract storage layout information
 */

#include <libsolidity/interface/StorageInfo.h>
#include <libsolidity/codegen/Compiler.h>
#include <libsolidity/ast/AST.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

Json::Value StorageInfo::generate(Compiler const* _compiler)
{
	Json::Value storage(Json::arrayValue);
	
	if(_compiler == NULL) 
	{
		return storage;
	}

	for (auto it: _compiler->stateVariables())
	{
		if (auto decl = dynamic_cast<VariableDeclaration const*>(it.first)) 
		{
			auto location = it.second;
			auto member = MemberList::Member(decl->name(), decl->type(), decl);
			auto memberData = processMember(member, location);
			
			// Assume that the parent scope of a state variable is a contract
			auto parent = ((Declaration*)decl->scope());
			if (parent != NULL) 
			{
				memberData["contract"] = parent->name();
			}

			storage.append(memberData);
		}
	}

	return storage;
}


Json::Value StorageInfo::processMember(MemberList::Member const& member, pair<u256, unsigned> const& location) 
{
	Json::Value data;
		
	data["name"] = member.name;
	data["slot"] = location.first.str();
	data["offset"] = to_string(location.second);
	data["type"] = processType(member.type);
	
	return data;
}


Json::Value StorageInfo::processType(TypePointer const& type) 
{
	Json::Value data;

	// Common type info
	data["name"] = type->canonicalName();
	data["size"] = type->storageSize().str();

	// Only include storageBytes if storageSize is 1, otherwise it always returns 32
	if (type->storageSize() == 1) 
	{
		data["bytes"] = to_string(type->storageBytes());
	}

	// Recursively visit complex types (structs, mappings, and arrays)
	if (type->category() == Type::Category::Struct) 
	{
		auto childStruct = static_pointer_cast<const StructType>(type);
		if (!childStruct->recursive())
		{
			Json::Value members(Json::arrayValue);
			for(auto member: childStruct->members(nullptr)) 
			{
				auto offsets = childStruct->storageOffsetsOfMember(member.name);
				auto memberData = processMember(member, offsets);
				members.append(memberData);
			}
			data["members"] = members;
		}
	} 
	else if (type->category() == Type::Category::Mapping) {
		auto map = static_pointer_cast<const MappingType>(type);
		data["key"] = processType(map->keyType());
		data["value"] = processType(map->valueType());
	}
	else if (type->category() == Type::Category::Array) {
		auto array = static_pointer_cast<const ArrayType>(type);
		if (!array->isByteArray())
		{
			data["base"] = processType(array->baseType());
		}
	}

	return data;
}