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
#include <libsolidity/lsp/References.h>
#include <libsolidity/lsp/ReferenceCollector.h>
#include <libsolidity/lsp/LanguageServer.h>
#include <libsolidity/lsp/Utils.h>

#include <libsolutil/CommonData.h>

using namespace std;
using namespace solidity::langutil;
using namespace solidity::frontend;

namespace solidity::lsp
{

void References::operator()(MessageID _id, Json::Value const& _args)
{
	auto const [sourceUnitName, lineColumn] = extractSourceUnitNameAndLineColumn(_args);

	ASTNode const* sourceNode = m_server.astNodeAtSourceLocation(sourceUnitName, lineColumn);
	SourceUnit const& sourceUnit = m_server.ast(sourceUnitName);

	Json::Value jsonReply = Json::arrayValue;
	for (auto const& reference: ReferenceCollector::collect(sourceNode, sourceUnit))
		jsonReply.append(toJson(get<SourceLocation>(reference)));

	client().reply(_id, jsonReply);
}

}
