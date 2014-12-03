#include <libsolidity/InterfaceHandler.h>
#include <libsolidity/AST.h>
#include <libsolidity/CompilerStack.h>

namespace dev {
namespace solidity {

InterfaceHandler::InterfaceHandler()
{
}

std::unique_ptr<std::string> InterfaceHandler::getDocumentation(std::shared_ptr<ContractDefinition> _contractDef,
												 enum documentation_type _type)
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

		auto streamVariables = [](std::vector<ASTPointer<VariableDeclaration>> const& _vars)
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
		method["inputs"] = streamVariables(f->getParameters());
		method["outputs"] = streamVariables(f->getReturnParameters());
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
	//TODO
	return nullptr;
}

} //solidity NS
} // dev NS
