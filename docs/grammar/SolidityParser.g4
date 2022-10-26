/**
 * Solidity是一种静态类型的，面向合约的高级语言，用于在Ethereum平台上实现智能合约。
 */
parser grammar SolidityParser;

options { tokenVocab=SolidityLexer; }

/**
 * 在顶层，Solidity允许pragmas，导入语句，
 * 以及合约，接口，库，结构，枚举和常量的定义。
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
 * 导入指令 从不同的文件中导入标识符。
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
 * 要导入的文件的路径。
 */
path: NonEmptyStringLiteral;
/**
 * 要导入的符号的别名列表。
 */
symbolAliases: LBrace aliases+=importAliases (Comma aliases+=importAliases)* RBrace;

/**
 * 合约的顶层定义。
 */
contractDefinition:
	Abstract? Contract name=identifier
	inheritanceSpecifierList?
	LBrace contractBodyElement* RBrace;
/**
 * 接口的顶层定义。
 */
interfaceDefinition:
	Interface name=identifier
	inheritanceSpecifierList?
	LBrace contractBodyElement* RBrace;
/**
 * 一个库合约的顶层定义。
 */
libraryDefinition: Library name=identifier LBrace contractBodyElement* RBrace;

//@doc:inline
inheritanceSpecifierList:
	Is inheritanceSpecifiers+=inheritanceSpecifier
	(Comma inheritanceSpecifiers+=inheritanceSpecifier)*?;
/**
 * 合约和接口的继承指定器。
 * 可以有选择地提供基本构造函数参数。
 */
inheritanceSpecifier: name=identifierPath arguments=callArgumentList?;

/**
 * 可以在合约，接口和库中使用的声明。
 *
 * 注意，接口和库不能包含构造函数，接口不能包含状态变量，
 * 库不能包含fallback，receive函数和非恒定状态变量。
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
 * 调用一个函数或类似的可调用对象时的参数。
 * 参数要么以逗号分隔的列表形式给出，要么以命名参数的映射形式给出。
 */
callArgumentList: LParen ((expression (Comma expression)*)? | LBrace (namedArgument (Comma namedArgument)*)? RBrace) RParen;
/**
 * 合格的名称。
 */
identifierPath: identifier (Period identifier)*;

/**
 * 对一个修改器的调用。如果修改器不需要参数，参数列表可以完全跳过（包括开头和结尾的括号）。
 */
modifierInvocation: identifierPath callArgumentList?;
/**
 * 函数和函数类型的可见性。
 */
visibility: Internal | External | Private | Public;
/**
 * 一个参数的列表，如函数参数或返回值。
 */
parameterList: parameters+=parameterDeclaration (Comma parameters+=parameterDeclaration)*;
//@doc:inline
parameterDeclaration: type=typeName location=dataLocation? name=identifier?;
/**
 * 一个构造函数的定义。
 * 必须始终提供一个实现。
 * 请注意，指定内部或公共可见性已被废弃。
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
 * 函数类型的状态可变性。
 * 如果没有指定可变性，则假定默认的可变性为 “非payable“。
 */
stateMutability: Pure | View | Payable;
/**
 *一个用于函数，修改器或状态变量的重载指定符。
 * 如果在被重载的几个基础合约中存在不明确的声明，
 * 必须给出一个完整的基础合约清单。
 */
overrideSpecifier: Override (LParen overrides+=identifierPath (Comma overrides+=identifierPath)* RParen)?;
/**
 * 合约，库和接口功能的定义。
 * 根据定义函数的上下文，可能会有进一步的限制。
 * 例如，接口中的函数必须是未实现的，也就是说，不能包含主体块。
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
 * 修改器的定义。
 * 注意，在修改器的主体块中，下划线不能作为标识符使用，
 * 而是作为占位符语句，用于修改器所应用的函数主体。
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
 * 特殊的fallback函数的定义。
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
 * 特殊的receive函数的定义。
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
 * 结构体的定义。可以出现在源代码单元的顶层，也可以出现在合约，库或接口中。
 */
structDefinition: Struct name=identifier LBrace members=structMember+ RBrace;
/**
 * 一个命名的结构体成员的声明。
 */
structMember: type=typeName name=identifier Semicolon;
/**
 * 一个枚举的定义。可以出现在源代码单元的顶层，也可以出现在合约，库或接口中。
 */
enumDefinition:	Enum name=identifier LBrace enumValues+=identifier (Comma enumValues+=identifier)* RBrace;
/**
 * 用户自定义的值类型的定义。可以出现在源代码单元的顶层，也可以出现在合约，库或接口中。
 */
userDefinedValueTypeDefinition:
	Type name=identifier Is elementaryTypeName[true] Semicolon;

/**
 * 一个状态变量的声明。
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
 * 一个常量变量的声明。
 */
constantVariableDeclaration
:
	type=typeName
	Constant
	name=identifier
	Assign initialValue=expression
	Semicolon;

/**
 * 一个事件类型的参数。
 */
eventParameter: type=typeName Indexed? name=identifier?;
/**
 * 一个事件类型的定义。可以发生在合约，库或接口中。
 */
eventDefinition:
	Event name=identifier
	LParen (parameters+=eventParameter (Comma parameters+=eventParameter)*)? RParen
	Anonymous?
	Semicolon;

/**
 * 一个错误类型的参数。
 */
errorParameter: type=typeName name=identifier?;
/**
 * 错误类型定义。
 */
errorDefinition:
	Error name=identifier
	LParen (parameters+=errorParameter (Comma parameters+=errorParameter)*)? RParen
	Semicolon;

/**
<<<<<<< HEAD
 * 使用指令将库函数与类型绑定。
 * 可以在合约和库中出现。
=======
 * Using directive to bind library functions and free functions to types.
 * Can occur within contracts and libraries and at the file level.
>>>>>>> 07a7930e73f57ce6ed1c6f0b8dd9aad99e5c3692
 */
usingDirective: Using (identifierPath | (LBrace identifierPath (Comma identifierPath)* RBrace)) For (Mul | typeName) Global? Semicolon;
/**
 * 一个类型名称可以是一个基本类型，一个函数类型，一个映射类型，
 * 一个用户定义的类型（如合约类型或结构体类型）或一个数组类型。
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
 * 单一变量的声明。
 */
variableDeclaration: type=typeName location=dataLocation? name=identifier;
dataLocation: Memory | Storage | Calldata;

/**
 * 复杂的表达式。
 * 可以是一个索引访问，一个索引范围访问，一个成员访问，一个函数调用（有可选的函数调用选项），
 * 一个类型转换，一个单数或双数表达式，一个比较或赋值，一个三元表达式，
 * 一个新的表达式（即一个合约的创建或动态内存数组的分配），
 * 一个元组，一个内联数组或一个主要表达式（即一个标识符，字面意思或类型名）。
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
 * 内联数组表达式表示一个静态大小的数组，它是所含表达式的共同类型。
 */
inlineArrayExpression: LBrack (expression ( Comma expression)* ) RBrack;

/**
 * 除了常规的非关键字标识符，一些关键字如 ‘from‘ 和 ‘error‘ 也可以作为标识符。
 */
identifier: Identifier | From | Error | Revert | Global;

literal: stringLiteral | numberLiteral | booleanLiteral | hexStringLiteral | unicodeStringLiteral;
booleanLiteral: True | False;
/**
 * 一个完整的字符串字面量由一个或几个连续的引号字符串组成。
 */
stringLiteral: (NonEmptyStringLiteral | EmptyStringLiteral)+;
/**
 * 一个完整的十六进制字符串字面量由一个或几个连续的十六进制字符串组成。
 */
hexStringLiteral: HexString+;
/**
 * 一个完整的unicode字符串字面量由一个或几个连续的unicode字符串组成。
 */
unicodeStringLiteral: UnicodeStringLiteral+;

/**
 * 数字字面量可以是带可选单位的十进制或十六进制数字。
 */
numberLiteral: (DecimalNumber | HexNumber) NumberUnit?;
/**
 * 带花括号的语句块。可以打开自己的作用域。
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
 * 带有可选的else部分的If语句。
 */
ifStatement: If LParen expression RParen statement (Else statement)?;
/**
 * 带有可选的初始值，循环条件和循环语句部分的For语句。
 */
forStatement: For LParen (simpleStatement | Semicolon) (expressionStatement | Semicolon) expression? RParen statement;
whileStatement: While LParen expression RParen statement;
doWhileStatement: Do statement While LParen expression RParen Semicolon;
/**
 * 一个continue语句。只允许在for、while或do-while循环中使用。
 */
continueStatement: Continue Semicolon;
/**
 * 一个break语句。只允许在for，while或do-while循环中使用。
 */
breakStatement: Break Semicolon;
/**
 * 一个try语句。包含的表达式需要是一个外部函数调用或合约创建。
 */
tryStatement: Try expression (Returns LParen returnParameters=parameterList RParen)? block catchClause+;
/**
 * Try语句的catch子句。
 */
catchClause: Catch (identifier? LParen (arguments=parameterList) RParen)? block;

returnStatement: Return expression? Semicolon;
/**
 * 一个发射语句。包含的表达式需要引用一个事件。
 */
emitStatement: Emit expression callArgumentList Semicolon;
/**
 * 一个恢复语句。包含的表达式需要指向一个错误。
 */
revertStatement: Revert expression callArgumentList Semicolon;
/**
 * 一个内联汇编代码块。
 * 内联汇编块的内容使用一个单独的扫描器/读取器，也就是说，内联汇编块内的关键字和允许的标识符集是不同的。
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
 * 在变量声明中使用的变量名元组。
 * 可能包含空字段。
 */
variableDeclarationTuple:
	LParen
		(Comma* variableDeclarations+=variableDeclaration)
		(Comma (variableDeclarations+=variableDeclaration)?)*
	RParen;
/**
 * 一个变量的声明语句。
 * 单个变量可以不带初始值声明，而变量的元组只能用初始值声明。
 */
variableDeclarationStatement: ((variableDeclaration (Assign expression)?) | (variableDeclarationTuple Assign expression)) Semicolon;
expressionStatement: expression Semicolon;

mappingType: Mapping LParen key=mappingKeyType DoubleArrow value=typeName RParen;
/**
 * 只有基本类型或用户定义的类型可以作为映射类型的键值。
 */
mappingKeyType: elementaryTypeName[false] | identifierPath;

/**
 * 内联汇编块中的Yul语句。
 * continue 和 break 语句只在for循环中有效。
 * 离开语句只在函数体内有效。
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
 * 声明一个或多个具有可选的初始值的Yul变量。
 * 如果声明了多个变量，只有一个函数调用是有效的初始值。
 */
yulVariableDeclaration:
	(YulLet variables+=YulIdentifier (YulAssign yulExpression)?)
	| (YulLet variables+=YulIdentifier (YulComma variables+=YulIdentifier)* (YulAssign yulFunctionCall)?);

/**
 * 任何表达式都可以分配给一个Yul变量，
 * 而多分配则需要在右侧调用一个函数。
 */
yulAssignment: yulPath YulAssign yulExpression | (yulPath (YulComma yulPath)+) YulAssign yulFunctionCall;

yulIfStatement: YulIf cond=yulExpression body=yulBlock;

yulForStatement: YulFor init=yulBlock cond=yulExpression post=yulBlock body=yulBlock;

//@doc:inline
yulSwitchCase: YulCase yulLiteral yulBlock;
/**
 * Yul switch语句可以只包括一个默认情况（已废弃）
 * 或一个或多个非默认情况，可选择紧跟一个默认情况。
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
 * 虽然只有不带点的标识符可以在内联汇编中声明，
 * 但含有点的路径可以指内联汇编块之外的声明。
 */
yulPath: YulIdentifier (YulPeriod (YulIdentifier | YulEVMBuiltin))*;
/**
 * 对带有返回值的函数的调用只能作为赋值或变量声明的右侧出现。
 */
yulFunctionCall: (YulIdentifier | YulEVMBuiltin) YulLParen (yulExpression (YulComma yulExpression)*)? YulRParen;
yulBoolean: YulTrue | YulFalse;
yulLiteral: YulDecimalNumber | YulStringLiteral | YulHexNumber | yulBoolean | YulHexStringLiteral;
yulExpression: yulPath | yulFunctionCall | yulLiteral;
