#include <libsolidity/InterfaceHandler.h>
#include <libsolidity/AST.h>
#include <libsolidity/CompilerStack.h>

namespace dev {
namespace solidity {

/* -- public -- */

InterfaceHandler::InterfaceHandler()
{
}

std::unique_ptr<std::string> InterfaceHandler::getDocumentation(std::shared_ptr<ContractDefinition> _contractDef,
																enum documentationType _type)
{
	switch(_type)
	{
	case NATSPEC_USER:
		return getUserDocumentation(_contractDef);
	case NATSPEC_DEV:
		return getDevDocumentation(_contractDef);
	case ABI_INTERFACE:
		return getABIInterface(_contractDef);
	}

	BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Internal error"));
	return nullptr;
}

std::unique_ptr<std::string> InterfaceHandler::getABIInterface(std::shared_ptr<ContractDefinition> _contractDef)
{
	Json::Value methods(Json::arrayValue);

	std::vector<FunctionDefinition const*> exportedFunctions = _contractDef->getInterfaceFunctions();
	for (FunctionDefinition const* f: exportedFunctions)
	{
		Json::Value method;
		Json::Value inputs(Json::arrayValue);
		Json::Value outputs(Json::arrayValue);

		auto populateParameters = [](std::vector<ASTPointer<VariableDeclaration>> const& _vars)
		{
			Json::Value params(Json::arrayValue);
			for (ASTPointer<VariableDeclaration> const& var: _vars)
			{
				Json::Value input;
				input["name"] = var->getName();
				input["type"] = var->getType()->toString();
				params.append(input);
			}
			return params;
		};

		method["name"] = f->getName();
		method["inputs"] = populateParameters(f->getParameters());
		method["outputs"] = populateParameters(f->getReturnParameters());
		methods.append(method);
	}
	return std::unique_ptr<std::string>(new std::string(m_writer.write(methods)));
}

std::unique_ptr<std::string> InterfaceHandler::getUserDocumentation(std::shared_ptr<ContractDefinition> _contractDef)
{
	Json::Value doc;
	Json::Value methods(Json::objectValue);

	for (FunctionDefinition const* f: _contractDef->getInterfaceFunctions())
	{
		Json::Value user;
		auto strPtr = f->getDocumentation();
		if (strPtr)
		{
			user["notice"] = Json::Value(*strPtr);
			methods[f->getName()] = user;
		}
	}
	doc["methods"] = methods;

	return std::unique_ptr<std::string>(new std::string(m_writer.write(doc)));
}

std::unique_ptr<std::string> InterfaceHandler::getDevDocumentation(std::shared_ptr<ContractDefinition> _contractDef)
{
	Json::Value doc;
	Json::Value methods(Json::objectValue);

	for (FunctionDefinition const* f: _contractDef->getInterfaceFunctions())
	{
		Json::Value method;
		auto strPtr = f->getDocumentation();
		if (strPtr)
		{
			m_dev.clear();
			parseDocString(*strPtr);

			method["dev"] = Json::Value(m_dev);
			methods[f->getName()] = method;
		}
	}
	doc["methods"] = methods;

	return std::unique_ptr<std::string>(new std::string(m_writer.write(doc)));
}

/* -- private -- */
size_t InterfaceHandler::parseDocTag(std::string const& _string, std::string const& _tag, size_t _pos)
{
	size_t nlPos = _pos;
	if (_tag == "dev")
	{
		nlPos = _string.find("\n", _pos);
		m_dev += _string.substr(_pos,
								nlPos == std::string::npos ?
								_string.length() :
								nlPos - _pos);
	}
	else if (_tag == "notice")
	{
		nlPos = _string.find("\n", _pos);
		m_notice += _string.substr(_pos,
								nlPos == std::string::npos ?
								_string.length() :
								nlPos - _pos);
	}
	else
	{
		//TODO: Some form of warning
	}

	return nlPos;
}

void InterfaceHandler::parseDocString(std::string const& _string, size_t _startPos)
{
	size_t pos2;
	size_t pos1 = _string.find("@", _startPos);

	if (pos1 == std::string::npos)
		return; // no doxytags found

	pos2 = _string.find(" ", pos1);
	if (pos2 == std::string::npos)
		return; //no end of tag found

	size_t newPos = parseDocTag(_string, _string.substr(pos1 + 1, pos2 - pos1), pos2);
	parseDocString(_string, newPos);
}

} //solidity NS
} // dev NS
