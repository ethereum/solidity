#include <libsolidity/ast/JsonInterfaceImporter.h>

using namespace std;

namespace solidity::frontend
{

ASTPointer<SourceUnit> JsonInterfaceImporter::importInterfaceAsSourceUnit(
	langutil::SourceLocation const& _location,
	std::optional<std::string> const& _license,
	std::string const& _name,
	Json::Value const& _source
)
{
	auto interface = importInterface(_location, _name, _source);
	return make_shared<SourceUnit>(
		m_nextId++,
		_location,
		_license,
		vector<ASTPointer<ASTNode>>{ interface }
	);
}

ASTPointer<ASTNode> JsonInterfaceImporter::importInterface(
	langutil::SourceLocation const& _location,
	string const& _name,
	Json::Value const& _source
)
{
	// TODO: contractInheritanceDefinition
	// TODO: "anything in ContractDefintion that returns a vector of FunctionDefinitons needs to be refactored to return a base class of FunctionDefiniton such that the imported interface can also return something that is not directly a FunctionDefinition"

	vector<ASTPointer<ASTNode>> members;
	for (auto const& jsonMember: _source)
		if (auto member = createMember(_location, jsonMember); member != nullptr)
			members.push_back(member);

	return make_shared<ContractDefinition>(
		m_nextId++,
		_location,
		make_shared<ASTString>(_name),
		ASTPointer<StructuredDocumentation>{},
		vector<ASTPointer<InheritanceSpecifier>>{}, // TODO: see contractInheritanceDefinition
		members,
		ContractKind::Interface,
		false
	);
}

ASTPointer<ASTNode> JsonInterfaceImporter::createMember(
	langutil::SourceLocation const& _location,
	Json::Value const& _node
)
{
	if (_node["type"] == "function")
		return createFunction(_location, _node);
	else if (_node["type"] == "event")
		return createEvent(_location, _node);
	else
	{
		// TODO: report error of invalid type. Use m_errorReporter for that.
		return nullptr;
	}
}

ASTPointer<ASTNode> JsonInterfaceImporter::createFunction(
	langutil::SourceLocation const& _location,
	Json::Value const& _node
)
{
	auto const mutability = stateMutability(_node["stateMutability"]);
	auto const name = _node["name"].asString();
	auto const inputs = createParameters(_location, false, _node["inputs"]);
	auto const outputs = createParameters(_location, false, _node["outputs"]);
	auto const kind = Token::Function;

	return createASTNode<FunctionDefinition>(
		_location,
		make_shared<ASTString>(name),
		Visibility::Default,
		mutability,
		kind,
		true,       // interface functions are always virtual
		nullptr,    // overrides
		nullptr,    // documentation
		inputs,
		vector<ASTPointer<ModifierInvocation>>{},
		outputs,
		nullptr		// no body
	);
}

ASTPointer<ASTNode> JsonInterfaceImporter::createEvent(
	langutil::SourceLocation const& _location,
	Json::Value const& _node
)
{
	bool const anonymous = _node["anonymous"].asBool();
	auto inputs = createParameters(_location, true, _node["inputs"]);
	auto const name = _node["name"].asString();

	return createASTNode<EventDefinition>(
		_location,
		make_shared<ASTString>(name),
		ASTPointer<StructuredDocumentation>{},
		inputs,
		anonymous
	);
}

ASTPointer<ParameterList> JsonInterfaceImporter::createParameters(
	langutil::SourceLocation const& _location,
	bool _indexed,
	Json::Value const& _node
)
{
	vector<ASTPointer<VariableDeclaration>> parameters;

	for (auto& param: _node)
		parameters.push_back(createParameter(_location, _indexed, param));

	return createASTNode<ParameterList>(_location, parameters);
}

ASTPointer<VariableDeclaration> JsonInterfaceImporter::createParameter(
	langutil::SourceLocation const& _location,
	bool _indexed,
	Json::Value const& _node
)
{
	// "indexed" (only when event)
	auto const parameterName = _node["name"].asString();
	auto const typeName = _node["type"].asString();
	ASTPointer<TypeName> type = {}; // TODO (get me from typeName)

	return createASTNode<VariableDeclaration>(
		_location,
		type,
		make_shared<ASTString>(parameterName),
		ASTPointer<Expression>{},			   // value
		Visibility::Default,
		ASTPointer<StructuredDocumentation>{}, // documentation
		false,                                 // isStateVariable
		_indexed,
		VariableDeclaration::Mutability::Mutable,
		ASTPointer<OverrideSpecifier>{}
	);
	// TODO
	// return createASTNode<VariableDeclaration>(
	// 	_location,
	// 	nullOrCast<TypeName>(member(_node, "typeName")),
	// 	make_shared<ASTString>(member(_node, "name").asString()),
	// 	nullOrCast<Expression>(member(_node, "value")),
	// 	visibility(_node),
	// 	_node["documentation"].isNull() ? nullptr : createDocumentation(member(_node, "documentation")),
	// 	memberAsBool(_node, "stateVariable"),
	// 	_node.isMember("indexed") ? memberAsBool(_node, "indexed") : false,
	// 	mutability,
	// 	_node["overrides"].isNull() ? nullptr : createOverrideSpecifier(member(_node, "overrides")),
	// 	location(_node)
	// );
}

StateMutability JsonInterfaceImporter::stateMutability(Json::Value const& _node) const
{
	// TODO: see that we can avoid duplication with ASTJsonImporter
	astAssert(member(_node, "stateMutability").isString(), "StateMutability' expected to be string.");
	string const mutabilityStr = member(_node, "stateMutability").asString();

	if (mutabilityStr == "pure")
		return StateMutability::Pure;
	else if (mutabilityStr == "view")
		return StateMutability::View;
	else if (mutabilityStr == "nonpayable")
		return StateMutability::NonPayable;
	else if (mutabilityStr == "payable")
		return StateMutability::Payable;
	else
		astAssert(false, "Unknown stateMutability");
}

Json::Value JsonInterfaceImporter::member(Json::Value const& _node, string const& _name) const
{
	// TODO: see that we can avoid duplication with ASTJsonImporter
	astAssert(_node.isMember(_name), "Node '" + _node["nodeType"].asString() + "' (id " + _node["id"].asString() + ") is missing field '" + _name + "'.");
	return _node[_name];
}

template <typename T, typename... Args>
ASTPointer<T> JsonInterfaceImporter::createASTNode(
	langutil::SourceLocation const& _location,
	Args&&... _args
)
{
	return make_shared<T>(m_nextId++, _location, forward<Args>(_args)...);
}

} // end namespace
