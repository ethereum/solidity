#pragma once

#include <map>

#include <libsolidity/Scope.h>
#include <libsolidity/ASTVisitor.h>

namespace dev {
namespace solidity {

class NameAndTypeResolver
{
public:
	NameAndTypeResolver();

	void resolveNamesAndTypes(ContractDefinition& _contract);
private:
	class ScopeHelper; //< RIIA helper to open and close scopes

	void reset();

	void handleContract(ContractDefinition& _contract);
	void handleFunction(FunctionDefinition& _function);
	void handleFunctionBody(Block& _functionBody);
	void registerVariablesInFunction(Block& _functionBody);
	void resolveReferencesInFunction(Block& _functionBody);

	void registerName(ASTString const& _name, ASTNode& _declaration);
	ASTNode* getNameFromCurrentScope(ASTString const& _name, bool _recursive = true);

	void enterNewSubScope(ASTNode& _node);
	void closeCurrentScope();

	Scope m_globalScope; // not part of the map
	std::map<ASTNode*, Scope> m_scopes;

	Scope* m_currentScope;
};

} }
