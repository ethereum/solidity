##########################
Abstract syntax tree (AST)
##########################

This guide documents the Solidity abstract syntax tree (AST), representing the abstract structure of code written in Solidity. It describes each node in the tree, parameters, and potential direct children.

.. TODO: Abstract classes from h file? Are they appended?

Common fields
-------------

All nodes share the same fields:

- ``Source``: Source code line(s) of the AST node.

Nodes
-----

PragmaDirective
===============

Pragma directive for the contract definition, only version requirements in the form `pragma solidity "^0.4.0";` are supported.

ImportDirective "file"
======================

Import directive for referencing other files / source objects.

ContractDefinition "ContractName"
=================================

The definition of a contract or library, followed by the name of the contract. This is the only AST node where Fields are not visited in document order. It first visits all struct declarations, then all variable declarations and finally all function declarations.

Fields
~~~~~~~~~~~

All other nodes.

UsingForDirective
=================

``using LibraryName for variable`` directive that attaches all functions from the library LibraryName the variable if the first parameter matches the type. ``using LibraryName for *`` attaches the function to any matching type.

Fields:

- ``UserDefinedTypeName``

CallableDeclaration
===================

Base class for all nodes that define function-like objects, i.e., ``FunctionDefinition``, ``EventDefinition`` and ``ModifierDefinition``.

Fields:

- ``ParameterList``
- ``Block``

FunctionDefinition "FunctionName"
=================================

Definition of a function and its name. Uses the same parameters and Fields as ``CallableDeclaration``.

EventDefinition "EventName"
===========================

Definition of an event and its name. Uses the same parameters and Fields as ``CallableDeclaration``.

ModifierDefinition "modifierName"
=================================

Definition of a function modifier and its name. Uses the same parameters and Fields as ``CallableDeclaration``.

ModifierInvocation
==================

.. TODO

ParameterList
=============

A list of function parameters and return variables. Does not include mappings.
.. TODO: And structs?

Fields:

- ``VariableDeclaration`` "VariableName" (multiple)

StructDefinition "VariableName"
===============================

Definition of a ``struct`` variable and its name.

Fields:

- ``VariableDeclaration`` "VariableName" (multiple)

EnumDefinition "VariableName"
===============================

Definition of a ``enum`` variable and its name.

Fields:

- ``EnumValue`` "VariableName" (multiple)

EnumValue
=========

Declaration of an Enum Value.

VariableDeclaration "VariableName"
==================================

Declaration of a variable and its name.

Fields:

``Type``: Variable type including any size

Fields:

- ``Mapping`` "MappingName"
- ``ElementaryTypeName`` "BaseType"
- ``ArrayTypeName``

Mapping "MappingName"
=====================

A mapping type and its name.

Fields:

- ``ElementaryTypeName`` "BaseType"
- ``UserDefinedTypeName`` "VariableName"

ElementaryTypeName "BaseType"
=============================

Any pre-defined type name represented by a single keyword (and state mutability for address types). Excludes mappings, contracts, functions, etc.

ArrayTypeName
=============

An array type that can be empty or contain an expression.

Fields:

- ``UserDefinedTypeName`` "VariableName"

MappingTypeName
===============

.. TODO: Actually exist?

FunctionTypeName
================

..  A literal function type. Its source form is "function (paramType1, paramType2) internal / external returns (retType1, retType2)"

.. TODO

InheritanceSpecifier
====================

UserDefinedTypeName "VariableName"
==================================

A user-defined type imported as another symbol name, or inherited from another contract. For example, ``import {symbol1 as alias, symbol2} from "filename"``, or ``contract mortal is owned``

Block
=====

Brace-enclosed block containing zero or more statements.

Fields:

- ``ExpressionStatement``
- ``ForStatement``
- ``EmitStatement``
- ``VariableDeclarationStatement``
- ``WhileStatement``
- ``Return``

ExpressionStatement
===================

A statement that contains one expression (i.e., an assignment, function call, etc.).

Fields:

- ``FunctionCall`` (multiple)
- ``UnaryOperation``
- Assignment

Assignment
==========

Assignment, can also be a compound assignment, e.g., ``(a = 7 + 8)`` or ``(a *= 2)``

Fields:

- Type

Fields:

- Identifier
- MemberAccess

EmitStatement
=============

The emit statement is used to emit events.

- Fields:

- ``FunctionCall``

FunctionCall
============

.. TODO: I don't really understand this

A function call, type cast or struct construction.

Fields:

- Identifier
- BinaryOperation
- Literal
- UnaryOperation
- ForStatement

.. TODO: maybe switch items after node type under heading in some way?

Return "identifier"
===================

Return statement to return a variable value.

Fields:

- Identifier

Break
=====

A break statement that terminates the current loop, switch, or label statement and transfers program control to the statement following the terminated statement.

Continue
========

A continue statement that terminates execution of the statement in the current iteration of the current loop, and continues execution of the loop with the next iteration.

Identifier "identifier"
=======================

An identifier, i.e., a reference to a declaration by name, such as a variable or function.

.. TODO: Are fields just nodes?

Fields:

- Type


UnaryOperation "(pre or postfix) operation"
===========================================

Operation involving a unary operator, pre- or postfix. For example: ``++i``, ``delete x`` or ``!true``

Fields:

- Type

Fields:

- Identifier

ForStatement
============

For loop statement.

Fields:

- VariableDeclarationStatement
- BinaryOperation
- ExpressionStatement
- Block

WhileStatement
==============

While loop statement.

- BinaryOperation
- Block

IfStatement
===========
If statement with an optional "else" part. Note that "else if" is modeled by having a new if statement as the false (else) body.

Fields:

-  ``BinaryOperation``
- ``Block``

VariableDeclarationStatement
============================

Definition of one or more variables as a statement inside a function. If multiple variables are declared, a value has to be assigned directly. If only a single variable is declared, the value can be missing.

Examples:

- ``uint[] memory a; uint a = 2;``
- ``(uint a, bytes32 b, ) = f();``
- ``(, uint a, , StructName storage x) = g();``

Fields:

- VariableDeclaration
- Literal

BinaryOperation
===============

Operation involving a binary operator. For example: ``1 + 2``, ``true && false`` or ``1 <= 4``.

Fields:

- Type

Fields:

- Identifier
- MemberAccess

Literal something?
==================

A literal string or number.

Fields:

- Type

MemberAccess something?
=======================

Access to a member of an object. For example: ``x.name``.

Fields:

- Type

Fields:

- IndexAccess

IndexAccess something?
======================

Index access to an array or mapping. For example: ``a[2]``.

Fields:

- Type

Fields:

- Identifier

NewExpression
=============

Expression that creates a new contract or memory-array, e.g., the "new SomeContract" part in "new SomeContract(1, 2)".

Fields:

- Type

Fields:

- UserDefinedTypeName

TupleExpression
===============

Tuple, parenthesized expression, or bracketed expression, e.g., (1, 2), (x,), (x), (), [1, 2]. Individual components might be empty shared pointers (as in the second example).

The respective types in lvalue context are: 2-tuple, 2-tuple (with wildcard), type of x, 0-tuple. Not in lvalue context: 2-tuple, _1_-tuple, type of x, 0-tuple.

Fields:

- Type

Fields:

- Literal

PrimaryExpression
=================

.. TODO

ElementaryTypeNameExpression
============================

.. TODO: Needed?

InlineAssembly
==============

Inline assembly.

.. TODO

Throw
=====

.. The Throw statement to throw that triggers a solidity exception(jump to ErrorTag)

.. TODO


---

The ``--ast-compact-json`` option generates an AST of all source files in a compact JSON format, useful for generating symbol tables for compilers and analysis tools.

The JSON output typically consists of an array of nodes (with an associated ``nodeType``) with key/value pair "fields". There are a lot of these, and depending on the contract structure, can have varying nested structures. You can see examples of the input contracts and their JSON AST output `in the Solidity test suite <https://github.com/ethereum/solidity/tree/develop/test/libsolidity/ASTJSON>`_, but below is an example with explanation.

**input**

::
    contract C { function f(function() external payable returns (uint) x) returns (function() external view returns (uint)) {} }

**output**

::

    {
    "absolutePath": "a",
    "exportedSymbols":
    {
        "C":
        [
        17
        ]
    },
    "id": 18,
    "nodeType": "SourceUnit",
    // Defines the type of node, including: 
    // SourceUnit, ContractDefinition, Block, FunctionDefinition, 
    // ParameterList, VariableDeclaration, FunctionTypeName, ElementaryTypeName
    "nodes":
    [
        {
        "abstract": false,
        "baseContracts": [], // Contracts this contract inherits from
        "contractDependencies": [], // Libraries this contract uses
        "contractKind": "contract", // Is this an interface, contract, or library
        "documentation": null,
        "fullyImplemented": true, // false if this is an abstract contract
        "id": 17,
        "linearizedBaseContracts":
        [
            17
        ],
        "name": "C",
        // User-defined name of the contract, function, library or variable
        "nodeType": "ContractDefinition",
        "nodes":
        [
            {
            "body": // Body of the node that likely contains further nodes
            {
                "id": 15,
                "nodeType": "Block",
                "src": "120:2:1",
                "statements": []
            },
            "documentation": null,
            "functionSelector": "d6cd4974",
            "id": 16,
            "implemented": true,
            "kind": "function",
            // Human readable node type
            "modifiers": [],
            "name": "f",
            "nodeType": "FunctionDefinition",
            "overrides": null,
            "parameters":
            {
                "id": 7,
                "nodeType": "ParameterList",
                "parameters":
                [
                {
                    "constant": false,
                    "id": 6,
                    "name": "x",
                    "nodeType": "VariableDeclaration",
                    "overrides": null,
                    "scope": 16,
                    "src": "24:44:1",
                    "stateVariable": false,
                    "storageLocation": "default",
                    // Memory storage location for variable
                    "typeDescriptions":
                    {
                    "typeIdentifier": "t_function_external_payable$__$returns$_t_uint256_$",
                    "typeString": "function () payable external returns (uint256)"
                    },
                    "typeName":
                    {
                    "id": 5,
                    "nodeType": "FunctionTypeName",
                    "parameterTypes":
                    {
                        "id": 1,
                        "nodeType": "ParameterList",
                        "parameters": [],
                        "src": "32:2:1"
                    },
                    "returnParameterTypes":
                    {
                        "id": 4,
                        "nodeType": "ParameterList",
                        "parameters":
                        [
                        {
                            "constant": false,
                            "id": 3,
                            "name": "",
                            "nodeType": "VariableDeclaration",
                            "overrides": null,
                            "scope": 5,
                            "src": "61:4:1",
                            "stateVariable": false,
                            "storageLocation": "default",
                            "typeDescriptions":
                            {
                            "typeIdentifier": "t_uint256",
                            "typeString": "uint256"
                            },
                            "typeName":
                            {
                            "id": 2,
                            "name": "uint",
                            "nodeType": "ElementaryTypeName",
                            "src": "61:4:1",
                            "typeDescriptions":
                            {
                                "typeIdentifier": "t_uint256",
                                "typeString": "uint256"
                            }
                            },
                            "value": null,
                            "visibility": "internal"
                            // Visibility of the function or variable
                        }
                        ],
                        "src": "60:6:1"
                    },
                    "src": "24:44:1",
                    "stateMutability": "payable",
                    "typeDescriptions":
                    {
                        "typeIdentifier": "t_function_external_payable$__$returns$_t_uint256_$",
                        "typeString": "function () payable external returns (uint256)"
                    },
                    "visibility": "external"
                    },
                    "value": null,
                    "visibility": "internal"
                }
                ],
                "src": "23:46:1"
            },
            "returnParameters":
            {
                "id": 14,
                "nodeType": "ParameterList",
                "parameters":
                [
                {
                    "constant": false,
                    "id": 13,
                    "name": "",
                    "nodeType": "VariableDeclaration",
                    "overrides": null,
                    "scope": 16,
                    "src": "79:40:1",
                    "stateVariable": false,
                    "storageLocation": "default",
                    "typeDescriptions":
                    {
                    "typeIdentifier": "t_function_external_view$__$returns$_t_uint256_$",
                    "typeString": "function () view external returns (uint256)"
                    },
                    "typeName":
                    {
                    "id": 12,
                    "nodeType": "FunctionTypeName",
                    "parameterTypes":
                    {
                        "id": 8,
                        "nodeType": "ParameterList",
                        "parameters": [],
                        "src": "87:2:1"
                    },
                    "returnParameterTypes":
                    {
                        "id": 11,
                        "nodeType": "ParameterList",
                        "parameters":
                        [
                        {
                            "constant": false,
                            "id": 10,
                            "name": "",
                            "nodeType": "VariableDeclaration",
                            "overrides": null,
                            "scope": 12,
                            "src": "113:4:1",
                            "stateVariable": false,
                            "storageLocation": "default",
                            "typeDescriptions":
                            {
                            "typeIdentifier": "t_uint256",
                            "typeString": "uint256"
                            },
                            "typeName":
                            {
                            "id": 9,
                            "name": "uint",
                            "nodeType": "ElementaryTypeName",
                            "src": "113:4:1",
                            "typeDescriptions":
                            {
                                "typeIdentifier": "t_uint256",
                                "typeString": "uint256"
                            }
                            },
                            "value": null,
                            "visibility": "internal"
                        }
                        ],
                        "src": "112:6:1"
                    },
                    "src": "79:40:1",
                    "stateMutability": "view",
                    "typeDescriptions":
                    {
                        "typeIdentifier": "t_function_external_view$__$returns$_t_uint256_$",
                        "typeString": "function () view external returns (uint256)"
                    },
                    "visibility": "external"
                    },
                    "value": null,
                    "visibility": "internal"
                }
                ],
                "src": "78:41:1"
            },
            "scope": 17,
            "src": "13:109:1",
            "stateMutability": "nonpayable",
            "virtual": false,
            "visibility": "public"
            }
        ],
        "scope": 18,
        "src": "0:124:1"
        }
    ],
    "src": "0:125:1"
    }





Fields:

- ``absolutePath``
- ``exportedSymbols``
- ``id``
- ``documentation``
- ``linearizedBaseContracts``
- ``src``:
- ``statements``
- implemented
- modifiers
- parameters
- ``constant``: Is parameter declared as a constant
- scope
- stateVariable
- typeDescriptions
    - typeIdentifier
    - typeString
- typeName
- returnParameterTypes
- value
- stateMutability