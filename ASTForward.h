#pragma once

#include <string>
#include <memory>
#include <vector>

// Forward-declare all AST node types

namespace dev {
namespace solidity {

class ASTNode;
class ContractDefinition;
class StructDefinition;
class ParameterList;
class FunctionDefinition;
class VariableDeclaration;
class TypeName;
class ElementaryTypeName;
class UserDefinedTypeName;
class Mapping;
class Statement;
class Block;
class IfStatement;
class BreakableStatement;
class WhileStatement;
class Continue;
class Break;
class Return;
class VariableDefinition;
class Expression;
class Assignment;
class UnaryOperation;
class BinaryOperation;
class FunctionCall;
class MemberAccess;
class IndexAccess;
class PrimaryExpression;
class Identifier;
class ElementaryTypeNameExpression;
class Literal;

// Used as pointers to AST nodes, to be replaced by more clever pointers, e.g. pointers which do
// not do reference counting but point to a special memory area that is completely released
// explicitly.
template <class T>
using ptr = std::shared_ptr<T>;
template <class T>
using vecptr = std::vector<ptr<T>>;

using ASTString = std::string;


} }
