// SPDX-License-Identifier: GPL-3.0
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Forward-declarations of AST classes.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

// Forward-declare all AST node types and related enums.

namespace solidity::langutil
{
enum class Token : unsigned int;
}

namespace solidity::frontend
{

class ASTNode;
class SourceUnit;
class PragmaDirective;
class ImportDirective;
class Declaration;
class CallableDeclaration;
class OverrideSpecifier;
class ContractDefinition;
class InheritanceSpecifier;
class UsingForDirective;
class StructDefinition;
class EnumDefinition;
class EnumValue;
class ParameterList;
class FunctionDefinition;
class VariableDeclaration;
class ModifierDefinition;
class ModifierInvocation;
class EventDefinition;
class MagicVariableDeclaration;
class TypeName;
class ElementaryTypeName;
class UserDefinedTypeName;
class FunctionTypeName;
class Mapping;
class ArrayTypeName;
class InlineAssembly;
class Statement;
class Block;
class PlaceholderStatement;
class IfStatement;
class TryCatchClause;
class TryStatement;
class BreakableStatement;
class WhileStatement;
class ForStatement;
class Continue;
class Break;
class Return;
class Throw;
class EmitStatement;
class VariableDeclarationStatement;
class ExpressionStatement;
class Expression;
class Conditional;
class Assignment;
class TupleExpression;
class UnaryOperation;
class BinaryOperation;
class FunctionCall;
class NewExpression;
class MemberAccess;
class IndexAccess;
class PrimaryExpression;
class Identifier;
class ElementaryTypeNameExpression;
class Literal;
class StructuredDocumentation;

class VariableScope;

// Used as pointers to AST nodes, to be replaced by more clever pointers, e.g. pointers which do
// not do reference counting but point to a special memory area that is completely released
// explicitly.
template <class T>
using ASTPointer = std::shared_ptr<T>;

using ASTString = std::string;

}
