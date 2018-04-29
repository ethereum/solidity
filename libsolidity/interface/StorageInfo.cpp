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

Json::Value StorageInfo::generate(Compiler const& _compiler)
{
	Json::Value storage(Json::arrayValue);

	for (auto it: _compiler.stateVariables())
	{
		if (auto decl = dynamic_cast<VariableDeclaration const*>(it.first)) 
		{
			auto location = it.second;	
			
			Json::Value variable;
			variable["name"] = decl->name();
			variable["slot"] = location.first.str();
			variable["offset"] = to_string(location.second);
			variable["type"] = decl->type()->canonicalName();
			variable["size"] = decl->type()->storageSize().str();

			// Only include storageBytes if storageSize is 1, otherwise it always returns 32
			if (decl->type()->storageSize() == 1) {
				variable["bytes"] = to_string(decl->type()->storageBytes());
			}
			
			// Assume that the parent scope of a state variable is a contract
			auto parent = ((Declaration*)decl->scope());
			if (parent != NULL) {
				variable["contract"] = parent->name();
			}
			
			storage.append(variable);
		}
	}

	return storage;
}