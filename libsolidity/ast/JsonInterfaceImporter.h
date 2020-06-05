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
#pragma once

#include <libsolidity/ast/AST.h>
#include <liblangutil/SourceLocation.h>

#include <json/json.h>

#include <optional>
#include <string>

namespace solidity::frontend
{

class JsonInterfaceImporter
{
public:
	JsonInterfaceImporter(int64_t _nextId):
		m_nextId{ _nextId }
	{}

	ASTPointer<SourceUnit> importInterfaceAsSourceUnit(
		langutil::SourceLocation const& _location,
		std::optional<std::string> const& _licenseString,
		std::string const& _name,
		Json::Value const& _source
	);

	ASTPointer<ASTNode> importInterface(
		langutil::SourceLocation const& _location,
		std::string const& _name,
		Json::Value const& _source
	);

private:
	ASTPointer<ASTNode> createMember(
		langutil::SourceLocation const& _location,
		Json::Value const& _node
	);

	ASTPointer<ASTNode> createFunction(
		langutil::SourceLocation const& _location,
		Json::Value const& _node
	);

	ASTPointer<ASTNode> createEvent(
		langutil::SourceLocation const& _location,
		Json::Value const& _node
	);

	ASTPointer<ParameterList> createParameters(
		langutil::SourceLocation const& _location,
		bool _indexed,
		Json::Value const& _node
	);

	ASTPointer<VariableDeclaration> createParameter(
		langutil::SourceLocation const& _location,
		bool _indexed,
		Json::Value const& _node
	);

private: // helper functions (TODO: that could be shared with ASTJsonImporter)
	StateMutability stateMutability(Json::Value const& _node) const;

	Json::Value member(Json::Value const& _node, std::string const& _name) const;

	template <typename T, typename... Args>
	ASTPointer<T> createASTNode(
		langutil::SourceLocation const& _location,
		Args&&... _args
	);

private:
	int64_t m_nextId;
};

} // end namespace
