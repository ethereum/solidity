#include <libsolidity/NameAndTypeResolver.h>

#include <libsolidity/AST.h>
#include <boost/assert.hpp>

namespace dev {
namespace solidity {


class NameAndTypeResolver::ScopeHelper {
public:
	ScopeHelper(NameAndTypeResolver& _resolver, ASTString const& _name, ASTNode& _declaration)
		: m_resolver(_resolver)
	{
		m_resolver.registerName(_name, _declaration);
		m_resolver.enterNewSubScope(_declaration);
	}
	~ScopeHelper()
	{
		m_resolver.closeCurrentScope();
	}

private:
	NameAndTypeResolver& m_resolver;
};


NameAndTypeResolver::NameAndTypeResolver()
{
}

void NameAndTypeResolver::resolveNamesAndTypes(ContractDefinition& _contract)
{
	reset();

	handleContract(_contract);
}

void NameAndTypeResolver::handleContract(ContractDefinition& _contract)
{
	ScopeHelper scopeHelper(*this, _contract.getName(), _contract);

	for (ptr<VariableDeclaration> const& variable : _contract.getStateVariables())
		registerName(variable->getName(), *variable);
	// @todo structs

	for (ptr<FunctionDefinition> const& function : _contract.getDefinedFunctions())
		handleFunction(*function);

	// @todo resolve names used in mappings
}

void NameAndTypeResolver::reset()
{
	m_scopes.clear();
	m_globalScope = Scope();
	m_currentScope = &m_globalScope;
}

void NameAndTypeResolver::handleFunction(FunctionDefinition& _function)
{
	ScopeHelper scopeHelper(*this, _function.getName(), _function);

	// @todo resolve names used in mappings
	for (ptr<VariableDeclaration> const& variable  : _function.getParameters())
		registerName(variable->getName(), *variable);
	if (_function.hasReturnParameters())
		for (ptr<VariableDeclaration> const& variable  : _function.getReturnParameters())
			registerName(variable->getName(), *variable);
	handleFunctionBody(_function.getBody());
}

void NameAndTypeResolver::handleFunctionBody(Block& _functionBody)
{
	registerVariablesInFunction(_functionBody);
	resolveReferencesInFunction(_functionBody);
}

void NameAndTypeResolver::registerVariablesInFunction(Block& _functionBody)
{
	class VariableDeclarationFinder : public ASTVisitor {
	public:
		VariableDeclarationFinder(NameAndTypeResolver& _resolver) : m_resolver(_resolver) {}
		virtual bool visit(VariableDeclaration& _variable) override {
			m_resolver.registerName(_variable.getName(), _variable);
			return false;
		}
	private:
		NameAndTypeResolver& m_resolver;
	};

	VariableDeclarationFinder declarationFinder(*this);
	_functionBody.accept(declarationFinder);
}

void NameAndTypeResolver::resolveReferencesInFunction(Block& _functionBody)
{
	class ReferencesResolver : public ASTVisitor {
	public:
		ReferencesResolver(NameAndTypeResolver& _resolver) : m_resolver(_resolver) {}
		virtual bool visit(Identifier& _identifier) override {
			ASTNode* node = m_resolver.getNameFromCurrentScope(_identifier.getName());
			if (node == nullptr)
				throw std::exception(); // @todo
			_identifier.setReferencedObject(*node);
			return false;
		}
	private:
		NameAndTypeResolver& m_resolver;
	};

	ReferencesResolver referencesResolver(*this);
	_functionBody.accept(referencesResolver);
}


void NameAndTypeResolver::registerName(ASTString const& _name, ASTNode& _declaration)
{
	if (!m_currentScope->registerName(_name, _declaration))
		throw std::exception(); // @todo
}

ASTNode* NameAndTypeResolver::getNameFromCurrentScope(ASTString const& _name, bool _recursive)
{
	return m_currentScope->resolveName(_name, _recursive);
}

void NameAndTypeResolver::enterNewSubScope(ASTNode& _node)
{
	decltype(m_scopes)::iterator iter;
	bool newlyAdded;
	std::tie(iter, newlyAdded) = m_scopes.emplace(&_node, Scope(m_currentScope));
	BOOST_ASSERT(newlyAdded);
	m_currentScope = &iter->second;
}

void NameAndTypeResolver::closeCurrentScope()
{
	m_currentScope = m_currentScope->getOuterScope();
}

} }
