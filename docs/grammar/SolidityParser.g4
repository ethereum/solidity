/**
 * Solidity is a statically typed, contract-oriented, high-level language for implementing smart contracts on the Ethereum platform.
 */
parser grammar SolidityParser;

options { tokenVocab=SolidityLexer; }

/**
 * On top level, Solidity allows pragmas, import directives, and
 * definitions of contracts, interfaces, libraries, structs, enums and constants.
 */
sourceUnit: (
	pragmaDirective
	| importDirective
	| usingDirective
	| contractDefinition
	| interfaceDefinition
	| libraryDefinition
	| functionDefinition
	| constantVariableDeclaration
	| structDefinition
	| enumDefinition
	| userDefinedValueTypeDefinition
	| errorDefinition
)* EOF;

//@doc: inline
pragmaDirective: Pragma PragmaToken+ PragmaSemicolon;

/**
 * Import directives import identifiers from different files.
 */
importDirective:
	Import (
		(path (As unitAlias=identifier)?)
		| (symbolAliases From path)
		| (Mul As unitAlias=identifier From path)
	) Semicolon;
//@doc: inline
//@doc:name aliases
importAliases: symbol=identifier (As alias=identifier)?;
/**
 * Path of a file to be imported.
 */
path: NonEmptyStringLiteral;
/**
 * List of aliases for symbols to be imported.
 */
symbolAliases: LBrace aliases+=importAliases (Comma aliases+=importAliases)* RBrace;

/**
 * Top-level definition of a contract.
 */
contractDefinition:
	Abstract? Contract name=identifier
	inheritanceSpecifierList?
	LBrace contractBodyElement* RBrace;
/**
 * Top-level definition of an interface.
 */
interfaceDefinition:
	Interface name=identifier
	inheritanceSpecifierList?
	LBrace contractBodyElement* RBrace;
/**
 * Top-level definition of a library.
 */
libraryDefinition: Library name=identifier LBrace contractBodyElement* RBrace;

//@doc:inline
inheritanceSpecifierList:
	Is inheritanceSpecifiers+=inheritanceSpecifier
	(Comma inheritanceSpecifiers+=inheritanceSpecifier)*?;
/**
 * Inheritance specifier for contracts and interfaces.
 * Can optionally supply base constructor arguments.
 */
inheritanceSpecifier: name=identifierPath arguments=callArgumentList?;

/**
 * Declarations that can be used in contracts, interfaces and libraries.
 *
 * Note that interfaces and libraries may not contain constructors, interfaces may not contain state variables
 * and libraries may not contain fallback, receive functions nor non-constant state variables.
 */
contractBodyElement:
	constructorDefinition
	| functionDefinition
	| modifierDefinition
	| fallbackFunctionDefinition
	| receiveFunctionDefinition
	| structDefinition
	| enumDefinition
	| userDefinedValueTypeDefinition
	| stateVariableDeclaration
	| eventDefinition
	| errorDefinition
	| usingDirective;
//@doc:inline
namedArgument: name=identifier Colon value=expression;
/**
 * Arguments when calling a function or a similar callable object.
 * The arguments are either given as comma separated list or as map of named arguments.
 */
callArgumentList: LParen ((expression (Comma expression)*)? | LBrace (namedArgument (Comma namedArgument)*)? RBrace) RParen;
/**
 * Qualified name.
 */
identifierPath: identifier (Period identifier)*;

/**
 * Call to a modifier. If the modifier takes no arguments, the argument list can be skipped entirely
 * (including opening and closing parentheses).
 */
modifierInvocation: identifierPath callArgumentList?;
/**
 * Visibility for functions and function types.
 */
visibility: Internal | External | Private | Public;
/**
 * A list of parameters, such as function arguments or return values.
 */
parameterList: parameters+=parameterDeclaration (Comma parameters+=parameterDeclaration)*;
//@doc:inline
parameterDeclaration: type=typeName location=dataLocation? name=identifier?;
/**
 * Definition of a constructor.
 * Must always supply an implementation.
 * Note that specifying internal or public visibility is deprecated.
 */
constructorDefinition
locals[boolean payableSet = false, boolean visibilitySet = false]
:
	Constructor LParen (arguments=parameterList)? RParen
	(
		modifierInvocation
		| {!$payableSet}? Payable {$payableSet = true;}
		| {!$visibilitySet}? Internal {$visibilitySet = true;}
		| {!$visibilitySet}? Public {$visibilitySet = true;}
	)*
	body=block;

/**
 * State mutability for function types.
 * The default mutability 'non-payable' is assumed if no mutability is specified.
 */
stateMutability: Pure | View | Payable;
/**
 * An override specifier used for functions, modifiers or state variables.
 * In cases where there are ambiguous declarations in several base contracts being overridden,
 * a complete list of base contracts has to be given.
 */
overrideSpecifier: Override (LParen overrides+=identifierPath (Comma overrides+=identifierPath)* RParen)?;
/**
 * The definition of contract, library and interface functions.
 * Depending on the context in which the function is defined, further restrictions may apply,
 * e.g. functions in interfaces have to be unimplemented, i.e. may not contain a body block.
 */
functionDefinition
locals[
	boolean visibilitySet = false,
	boolean mutabilitySet = false,
	boolean virtualSet = false,
	boolean overrideSpecifierSet = false
]
:
	Function (identifier | Fallback | Receive)
	LParen (arguments=parameterList)? RParen
	(
		{!$visibilitySet}? visibility {$visibilitySet = true;}
		| {!$mutabilitySet}? stateMutability {$mutabilitySet = true;}
		| modifierInvocation
		| {!$virtualSet}? Virtual {$virtualSet = true;}
		| {!$overrideSpecifierSet}? overrideSpecifier {$overrideSpecifierSet = true;}
	 )*
	(Returns LParen returnParameters=parameterList RParen)?
	(Semicolon | body=block);
/**
 * The definition of a modifier.
 * Note that within the body block of a modifier, the underscore cannot be used as identifier,
 * but is used as placeholder statement for the body of a function to which the modifier is applied.
 */
modifierDefinition
locals[
	boolean virtualSet = false,
	boolean overrideSpecifierSet = false
]
:
	Modifier name=identifier
	(LParen (arguments=parameterList)? RParen)?
	(
		{!$virtualSet}? Virtual {$virtualSet = true;}
		| {!$overrideSpecifierSet}? overrideSpecifier {$overrideSpecifierSet = true;}
	)*
	(Semicolon | body=block);

/**
 * Definition of the special fallback function.
 */
fallbackFunctionDefinition
locals[
	boolean visibilitySet = false,
	boolean mutabilitySet = false,
	boolean virtualSet = false,
	boolean overrideSpecifierSet = false,
	boolean hasParameters = false
]
:
	kind=Fallback LParen (parameterList { $hasParameters = true; } )? RParen
	(
		{!$visibilitySet}? External {$visibilitySet = true;}
		| {!$mutabilitySet}? stateMutability {$mutabilitySet = true;}
		| modifierInvocation
		| {!$virtualSet}? Virtual {$virtualSet = true;}
		| {!$overrideSpecifierSet}? overrideSpecifier {$overrideSpecifierSet = true;}
	)*
	( {$hasParameters}? Returns LParen returnParameters=parameterList RParen | {!$hasParameters}? )
	(Semicolon | body=block);

/**
 * Definition of the special receive function.
 */
receiveFunctionDefinition
locals[
	boolean visibilitySet = false,
	boolean mutabilitySet = false,
	boolean virtualSet = false,
	boolean overrideSpecifierSet = false
]
:
	kind=Receive LParen RParen
	(
		{!$visibilitySet}? External {$visibilitySet = true;}
		| {!$mutabilitySet}? Payable {$mutabilitySet = true;}
		| modifierInvocation
		| {!$virtualSet}? Virtual {$virtualSet = true;}
		| {!$overrideSpecifierSet}? overrideSpecifier {$overrideSpecifierSet = true;}
	 )*
	(Semicolon | body=block);

/**
 * Definition of a struct. Can occur at top-level within a source unit or within a contract, library or interface.
 */
structDefinition: Struct name=identifier LBrace members=structMember+ RBrace;
/**
 * The declaration of a named struct member.
 */
structMember: type=typeName name=identifier Semicolon;
/**
 * Definition of an enum. Can occur at top-level within a source unit or within a contract, library or interface.
 */
enumDefinition:	Enum name=identifier LBrace enumValues+=identifier (Comma enumValues+=identifier)* RBrace;
/**
 * Definition of a user defined value type. Can occur at top-level within a source unit or within a contract, library or interface.
 */
userDefinedValueTypeDefinition:
	Type name=identifier Is elementaryTypeName[true] Semicolon;

/**
 * The declaration of a state variable.
 */
stateVariableDeclaration
locals [boolean constantnessSet = false, boolean visibilitySet = false, boolean overrideSpecifierSet = false]
:
	type=typeName
	(
		{!$visibilitySet}? Public {$visibilitySet = true;}
		| {!$visibilitySet}? Private {$visibilitySet = true;}
		| {!$visibilitySet}? Internal {$visibilitySet = true;}
		| {!$constantnessSet}? Constant {$constantnessSet = true;}
		| {!$overrideSpecifierSet}? overrideSpecifier {$overrideSpecifierSet = true;}
		| {!$constantnessSet}? Immutable {$constantnessSet = true;}
	)*
	name=identifier
	(Assign initialValue=expression)?
	Semicolon;

/**
 * The declaration of a constant variable.
 */
constantVariableDeclaration
:
	type=typeName
	Constant
	name=identifier
	Assign initialValue=expression
	Semicolon;

/**
 * Parameter of an event.
 */
eventParameter: type=typeName Indexed? name=identifier?;
/**
 * Definition of an event. Can occur in contracts, libraries or interfaces.
 */
eventDefinition:
	Event name=identifier
	LParen (parameters+=eventParameter (Comma parameters+=eventParameter)*)? RParen
	Anonymous?
	Semicolon;

/**
 * Parameter of an error.
 */
errorParameter: type=typeName name=identifier?;
/**
 * Definition of an error.
 */
errorDefinition:
	Error name=identifier
	LParen (parameters+=errorParameter (Comma parameters+=errorParameter)*)? RParen
	Semicolon;

/**
 * User type operators.
 */
userOp: Add | Sub | Mul | Div | Mod | BitAnd | BitOr | BitXor | BitNot | Equal | NotEqual | LessThan | GreaterThan | LessThanOrEqual | GreaterThanOrEqual | Shl | Sar | Exp | Not;

/**
 * Using directive to bind library functions and free functions to types.
 * Can occur within contracts and libraries and at the file level.
 */
usingDirective: Using (identifierPath (As userOp)? | (LBrace identifierPath (As userOp)? (Comma identifierPath (As userOp)?)* RBrace)) For (Mul | typeName) Global? Semicolon;
/**
 * A type name can be an elementary type, a function type, a mapping type, a user-defined type
 * (e.g. a contract or struct) or an array type.
 */
typeName: elementaryTypeName[true] | functionTypeName | mappingType | identifierPath | typeName LBrack expression? RBrack;
elementaryTypeName[boolean allowAddressPayable]: Address | {$allowAddressPayable}? Address Payable | Bool | String | Bytes | SignedIntegerType | UnsignedIntegerType | FixedBytes | Fixed | Ufixed;
functionTypeName
locals [boolean visibilitySet = false, boolean mutabilitySet = false]
:
	Function LParen (arguments=parameterList)? RParen
	(
		{!$visibilitySet}? visibility {$visibilitySet = true;}
		| {!$mutabilitySet}? stateMutability {$mutabilitySet = true;}
	)*
	(Returns LParen returnParameters=parameterList RParen)?;

/**
 * The declaration of a single variable.
 */
variableDeclaration: type=typeName location=dataLocation? name=identifier;
dataLocation: Memory | Storage | Calldata;

/**
 * Complex expression.
 * Can be an index access, an index range access, a member access, a function call (with optional function call options),
 * a type conversion, an unary or binary expression, a comparison or assignment, a ternary expression,
 * a new-expression (i.e. a contract creation or the allocation of a dynamic memory array),
 * a tuple, an inline array or a primary expression (i.e. an identifier, literal or type name).
 */
expression:
	expression LBrack index=expression? RBrack # IndexAccess
	| expression LBrack start=expression? Colon end=expression? RBrack # IndexRangeAccess
	| expression Period (identifier | Address) # MemberAccess
	| expression LBrace (namedArgument (Comma namedArgument)*)? RBrace # FunctionCallOptions
	| expression callArgumentList # FunctionCall
	| Payable callArgumentList # PayableConversion
	| Type LParen typeName RParen # MetaType
	| (Inc | Dec | Not | BitNot | Delete | Sub) expression # UnaryPrefixOperation
	| expression (Inc | Dec) # UnarySuffixOperation
	|<assoc=right> expression Exp expression # ExpOperation
	| expression (Mul | Div | Mod) expression # MulDivModOperation
	| expression (Add | Sub) expression # AddSubOperation
	| expression (Shl | Sar | Shr) expression # ShiftOperation
	| expression BitAnd expression # BitAndOperation
	| expression BitXor expression # BitXorOperation
	| expression BitOr expression # BitOrOperation
	| expression (LessThan | GreaterThan | LessThanOrEqual | GreaterThanOrEqual) expression # OrderComparison
	| expression (Equal | NotEqual) expression # EqualityComparison
	| expression And expression # AndOperation
	| expression Or expression # OrOperation
	|<assoc=right> expression Conditional expression Colon expression # Conditional
	|<assoc=right> expression assignOp expression # Assignment
	| New typeName # NewExpression
	| tupleExpression # Tuple
	| inlineArrayExpression # InlineArray
 	| (
		identifier
		| literal
		| elementaryTypeName[false]
	  ) # PrimaryExpression
;

//@doc:inline
assignOp: Assign | AssignBitOr | AssignBitXor | AssignBitAnd | AssignShl | AssignSar | AssignShr | AssignAdd | AssignSub | AssignMul | AssignDiv | AssignMod;
tupleExpression: LParen (expression? ( Comma expression?)* ) RParen;
/**
 * An inline array expression denotes a statically sized array of the common type of the contained expressions.
 */
inlineArrayExpression: LBrack (expression ( Comma expression)* ) RBrack;

/**
 * Besides regular non-keyword Identifiers, some keywords like 'from' and 'error' can also be used as identifiers.
 */
identifier: Identifier | From | Error | Revert | Global;

literal: stringLiteral | numberLiteral | booleanLiteral | hexStringLiteral | unicodeStringLiteral;
booleanLiteral: True | False;
/**
 * A full string literal consists of either one or several consecutive quoted strings.
 */
stringLiteral: (NonEmptyStringLiteral | EmptyStringLiteral)+;
/**
 * A full hex string literal that consists of either one or several consecutive hex strings.
 */
hexStringLiteral: HexString+;
/**
 * A full unicode string literal that consists of either one or several consecutive unicode strings.
 */
unicodeStringLiteral: UnicodeStringLiteral+;

/**
 * Number literals can be decimal or hexadecimal numbers with an optional unit.
 */
numberLiteral: (DecimalNumber | HexNumber) NumberUnit?;
/**
 * A curly-braced block of statements. Opens its own scope.
 */
block:
	LBrace ( statement | uncheckedBlock )* RBrace;

uncheckedBlock: Unchecked block;

statement:
	block
	| simpleStatement
	| ifStatement
	| forStatement
	| whileStatement
	| doWhileStatement
	| continueStatement
	| breakStatement
	| tryStatement
	| returnStatement
	| emitStatement
	| revertStatement
	| assemblyStatement
;

//@doc:inline
simpleStatement: variableDeclarationStatement | expressionStatement;
/**
 * If statement with optional else part.
 */
ifStatement: If LParen expression RParen statement (Else statement)?;
/**
 * For statement with optional init, condition and post-loop part.
 */
forStatement: For LParen (simpleStatement | Semicolon) (expressionStatement | Semicolon) expression? RParen statement;
whileStatement: While LParen expression RParen statement;
doWhileStatement: Do statement While LParen expression RParen Semicolon;
/**
 * A continue statement. Only allowed inside for, while or do-while loops.
 */
continueStatement: Continue Semicolon;
/**
 * A break statement. Only allowed inside for, while or do-while loops.
 */
breakStatement: Break Semicolon;
/**
 * A try statement. The contained expression needs to be an external function call or a contract creation.
 */
tryStatement: Try expression (Returns LParen returnParameters=parameterList RParen)? block catchClause+;
/**
 * The catch clause of a try statement.
 */
catchClause: Catch (identifier? LParen (arguments=parameterList) RParen)? block;

returnStatement: Return expression? Semicolon;
/**
 * An emit statement. The contained expression needs to refer to an event.
 */
emitStatement: Emit expression callArgumentList Semicolon;
/**
 * A revert statement. The contained expression needs to refer to an error.
 */
revertStatement: Revert expression callArgumentList Semicolon;
/**
 * An inline assembly block.
 * The contents of an inline assembly block use a separate scanner/lexer, i.e. the set of keywords and
 * allowed identifiers is different inside an inline assembly block.
 */
assemblyStatement: Assembly AssemblyDialect? assemblyFlags? AssemblyLBrace yulStatement* YulRBrace;

/**
 * Assembly flags.
 * Comma-separated list of double-quoted strings as flags.
 */
assemblyFlags: AssemblyBlockLParen AssemblyFlagString (AssemblyBlockComma AssemblyFlagString)* AssemblyBlockRParen;

//@doc:inline
variableDeclarationList: variableDeclarations+=variableDeclaration (Comma variableDeclarations+=variableDeclaration)*;
/**
 * A tuple of variable names to be used in variable declarations.
 * May contain empty fields.
 */
variableDeclarationTuple:
	LParen
		(Comma* variableDeclarations+=variableDeclaration)
		(Comma (variableDeclarations+=variableDeclaration)?)*
	RParen;
/**
 * A variable declaration statement.
 * A single variable may be declared without initial value, whereas a tuple of variables can only be
 * declared with initial value.
 */
variableDeclarationStatement: ((variableDeclaration (Assign expression)?) | (variableDeclarationTuple Assign expression)) Semicolon;
expressionStatement: expression Semicolon;

mappingType: Mapping LParen key=mappingKeyType DoubleArrow value=typeName RParen;
/**
 * Only elementary types or user defined types are viable as mapping keys.
 */
mappingKeyType: elementaryTypeName[false] | identifierPath;

/**
 * A Yul statement within an inline assembly block.
 * continue and break statements are only valid within for loops.
 * leave statements are only valid within function bodies.
 */
yulStatement:
	yulBlock
	| yulVariableDeclaration
	| yulAssignment
	| yulFunctionCall
	| yulIfStatement
	| yulForStatement
	| yulSwitchStatement
	| YulLeave
	| YulBreak
	| YulContinue
	| yulFunctionDefinition;

yulBlock: YulLBrace yulStatement* YulRBrace;

/**
 * The declaration of one or more Yul variables with optional initial value.
 * If multiple variables are declared, only a function call is a valid initial value.
 */
yulVariableDeclaration:
	(YulLet variables+=YulIdentifier (YulAssign yulExpression)?)
	| (YulLet variables+=YulIdentifier (YulComma variables+=YulIdentifier)* (YulAssign yulFunctionCall)?);

/**
 * Any expression can be assigned to a single Yul variable, whereas
 * multi-assignments require a function call on the right-hand side.
 */
yulAssignment: yulPath YulAssign yulExpression | (yulPath (YulComma yulPath)+) YulAssign yulFunctionCall;

yulIfStatement: YulIf cond=yulExpression body=yulBlock;

yulForStatement: YulFor init=yulBlock cond=yulExpression post=yulBlock body=yulBlock;

//@doc:inline
yulSwitchCase: YulCase yulLiteral yulBlock;
/**
 * A Yul switch statement can consist of only a default-case (deprecated) or
 * one or more non-default cases optionally followed by a default-case.
 */
yulSwitchStatement:
	YulSwitch yulExpression
	(
		(yulSwitchCase+ (YulDefault yulBlock)?)
		| (YulDefault yulBlock)
	);

yulFunctionDefinition:
	YulFunction YulIdentifier
	YulLParen (arguments+=YulIdentifier (YulComma arguments+=YulIdentifier)*)? YulRParen
	(YulArrow returnParameters+=YulIdentifier (YulComma returnParameters+=YulIdentifier)*)?
	body=yulBlock;

/**
 * While only identifiers without dots can be declared within inline assembly,
 * paths containing dots can refer to declarations outside the inline assembly block.
 */
yulPath: YulIdentifier (YulPeriod (YulIdentifier | YulEVMBuiltin))*;
/**
 * A call to a function with return values can only occur as right-hand side of an assignment or
 * a variable declaration.
 */
yulFunctionCall: (YulIdentifier | YulEVMBuiltin) YulLParen (yulExpression (YulComma yulExpression)*)? YulRParen;
yulBoolean: YulTrue | YulFalse;
yulLiteral: YulDecimalNumber | YulStringLiteral | YulHexNumber | yulBoolean | YulHexStringLiteral;
yulExpression: yulPath | yulFunctionCall | yulLiteral;
