#include <liblangutil/CharStreamProvider.h>
#include <liblangutil/Exceptions.h>
#include <libsolidity/ast/AST.h>
#include <libsolidity/lsp/FileRepository.h>
#include <libsolidity/lsp/Utils.h>

namespace solidity::lsp
{

using namespace frontend;
using namespace langutil;
using namespace std;

optional<LineColumn> parseLineColumn(Json::Value const& _lineColumn)
{
	if (_lineColumn.isObject() && _lineColumn["line"].isInt() && _lineColumn["character"].isInt())
		return LineColumn{_lineColumn["line"].asInt(), _lineColumn["character"].asInt()};
	else
		return nullopt;
}

Json::Value toJson(LineColumn _pos)
{
	Json::Value json = Json::objectValue;
	json["line"] = max(_pos.line, 0);
	json["character"] = max(_pos.column, 0);

	return json;
}

Json::Value toJsonRange(LineColumn const& _start, LineColumn const& _end)
{
	Json::Value json;
	json["start"] = toJson(_start);
	json["end"] = toJson(_end);
	return json;
}

vector<Declaration const*> allAnnotatedDeclarations(Expression const* _expression)
{
	vector<Declaration const*> output;

	if (auto const* identifier = dynamic_cast<Identifier const*>(_expression))
	{
		output.push_back(identifier->annotation().referencedDeclaration);
		output += identifier->annotation().candidateDeclarations;
	}
	else if (auto const* memberAccess = dynamic_cast<MemberAccess const*>(_expression))
	{
		output.push_back(memberAccess->annotation().referencedDeclaration);
	}

	return output;
}

optional<SourceLocation> declarationPosition(Declaration const* _declaration)
{
	if (!_declaration)
		return nullopt;

	if (_declaration->nameLocation().isValid())
		return _declaration->nameLocation();

	if (_declaration->location().isValid())
		return _declaration->location();

	return nullopt;
}

}
