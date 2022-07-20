lexer grammar SolidityLexer;

/**
 * 为将来在Solidity中使用保留的关键字。
 */
ReservedKeywords:
	'after' | 'alias' | 'apply' | 'auto' | 'byte' | 'case' | 'copyof' | 'default' | 'define' | 'final'
	| 'implements' | 'in' | 'inline' | 'let' | 'macro' | 'match' | 'mutable' | 'null' | 'of'
	| 'partial' | 'promise' | 'reference' | 'relocatable' | 'sealed' | 'sizeof' | 'static'
	| 'supports' | 'switch' | 'typedef' | 'typeof' | 'var';

Pragma: 'pragma' -> pushMode(PragmaMode);
Abstract: 'abstract';
Anonymous: 'anonymous';
Address: 'address';
As: 'as';
Assembly: 'assembly' -> pushMode(AssemblyBlockMode);
Bool: 'bool';
Break: 'break';
Bytes: 'bytes';
Calldata: 'calldata';
Catch: 'catch';
Constant: 'constant';
Constructor: 'constructor';
Continue: 'continue';
Contract: 'contract';
Delete: 'delete';
Do: 'do';
Else: 'else';
Emit: 'emit';
Enum: 'enum';
Error: 'error'; // 不是真正的关键字
Revert: 'revert'; // 不是真正的关键字
Event: 'event';
External: 'external';
Fallback: 'fallback';
False: 'false';
Fixed: 'fixed' | ('fixed' [1-9][0-9]* 'x' [1-9][0-9]*);
From: 'from'; // 不是真正的关键字
/**
 * 固定长度的字节类型。
 */
FixedBytes:
	'bytes1' | 'bytes2' | 'bytes3' | 'bytes4' | 'bytes5' | 'bytes6' | 'bytes7' | 'bytes8' |
	'bytes9' | 'bytes10' | 'bytes11' | 'bytes12' | 'bytes13' | 'bytes14' | 'bytes15' | 'bytes16' |
	'bytes17' | 'bytes18' | 'bytes19' | 'bytes20' | 'bytes21' | 'bytes22' | 'bytes23' | 'bytes24' |
	'bytes25' | 'bytes26' | 'bytes27' | 'bytes28' | 'bytes29' | 'bytes30' | 'bytes31' | 'bytes32';
For: 'for';
Function: 'function';
Hex: 'hex';
If: 'if';
Immutable: 'immutable';
Import: 'import';
Indexed: 'indexed';
Interface: 'interface';
Internal: 'internal';
Is: 'is';
Library: 'library';
Mapping: 'mapping';
Memory: 'memory';
Modifier: 'modifier';
New: 'new';
/**
 * Unit denomination for numbers.
 */
NumberUnit: 'wei' | 'gwei' | 'ether' | 'seconds' | 'minutes' | 'hours' | 'days' | 'weeks' | 'years';
Override: 'override';
Payable: 'payable';
Private: 'private';
Public: 'public';
Pure: 'pure';
Receive: 'receive';
Return: 'return';
Returns: 'returns';
/**
 * 有符号的整数类型。
 * int是int256的一个别名。
 */
SignedIntegerType:
	'int' | 'int8' | 'int16' | 'int24' | 'int32' | 'int40' | 'int48' | 'int56' | 'int64' |
	'int72' | 'int80' | 'int88' | 'int96' | 'int104' | 'int112' | 'int120' | 'int128' |
	'int136' | 'int144' | 'int152' | 'int160' | 'int168' | 'int176' | 'int184' | 'int192' |
	'int200' | 'int208' | 'int216' | 'int224' | 'int232' | 'int240' | 'int248' | 'int256';
Storage: 'storage';
String: 'string';
Struct: 'struct';
True: 'true';
Try: 'try';
Type: 'type';
Ufixed: 'ufixed' | ('ufixed' [1-9][0-9]+ 'x' [1-9][0-9]+);
Unchecked: 'unchecked';
/**
 * 无符号整数类型。
 * uint是uint256的一个别名。
 */
UnsignedIntegerType:
	'uint' | 'uint8' | 'uint16' | 'uint24' | 'uint32' | 'uint40' | 'uint48' | 'uint56' | 'uint64' |
	'uint72' | 'uint80' | 'uint88' | 'uint96' | 'uint104' | 'uint112' | 'uint120' | 'uint128' |
	'uint136' | 'uint144' | 'uint152' | 'uint160' | 'uint168' | 'uint176' | 'uint184' | 'uint192' |
	'uint200' | 'uint208' | 'uint216' | 'uint224' | 'uint232' | 'uint240' | 'uint248' | 'uint256';
Using: 'using';
View: 'view';
Virtual: 'virtual';
While: 'while';

LParen: '(';
RParen: ')';
LBrack: '[';
RBrack: ']';
LBrace: '{';
RBrace: '}';
Colon: ':';
Semicolon: ';';
Period: '.';
Conditional: '?';
DoubleArrow: '=>';
RightArrow: '->';

Assign: '=';
AssignBitOr: '|=';
AssignBitXor: '^=';
AssignBitAnd: '&=';
AssignShl: '<<=';
AssignSar: '>>=';
AssignShr: '>>>=';
AssignAdd: '+=';
AssignSub: '-=';
AssignMul: '*=';
AssignDiv: '/=';
AssignMod: '%=';

Comma: ',';
Or: '||';
And: '&&';
BitOr: '|';
BitXor: '^';
BitAnd: '&';
Shl: '<<';
Sar: '>>';
Shr: '>>>';
Add: '+';
Sub: '-';
Mul: '*';
Div: '/';
Mod: '%';
Exp: '**';

Equal: '==';
NotEqual: '!=';
LessThan: '<';
GreaterThan: '>';
LessThanOrEqual: '<=';
GreaterThanOrEqual: '>=';
Not: '!';
BitNot: '~';
Inc: '++';
Dec: '--';
//@doc:inline
DoubleQuote: '"';
//@doc:inline
SingleQuote: '\'';

/**
 * 一个非空的带引号的字符串字面量，限制为可打印的字符。
 */
NonEmptyStringLiteral: '"' DoubleQuotedStringCharacter+ '"' | '\'' SingleQuotedStringCharacter+ '\'';
/**
 * 一个空的字符串字面量
 */
EmptyStringLiteral: '"' '"' | '\'' '\'';

// 请注意，这也将被用于Yul字符串字面量。
//@doc:inline
fragment DoubleQuotedStringCharacter: DoubleQuotedPrintable | EscapeSequence;
// 请注意，这也将被用于Yul字符串字面量。
//@doc:inline
fragment SingleQuotedStringCharacter: SingleQuotedPrintable | EscapeSequence;
/**
 * Any printable character except single quote or back slash.
 */
fragment SingleQuotedPrintable: [\u0020-\u0026\u0028-\u005B\u005D-\u007E];
/**
 * 除双引号或反斜线外的任何可打印的字符。
 */
fragment DoubleQuotedPrintable: [\u0020-\u0021\u0023-\u005B\u005D-\u007E];
/**
  * 转义序列。
  * 除了常见的单字符转义序列外，还可以转义换行，
  * 以及允许四个十六进制数字的unicode转义\\uXXXX和两个十六进制数字的转义序列\\xXX。
  */
fragment EscapeSequence:
	'\\' (
		['"\\nrt\n\r]
		| 'u' HexCharacter HexCharacter HexCharacter HexCharacter
		| 'x' HexCharacter HexCharacter
	);
/**
 * 单引号字符串字面量，允许任意的unicode字符。
 */
UnicodeStringLiteral:
	'unicode"' DoubleQuotedUnicodeStringCharacter* '"'
	| 'unicode\'' SingleQuotedUnicodeStringCharacter* '\'';
//@doc:inline
fragment DoubleQuotedUnicodeStringCharacter: ~["\r\n\\] | EscapeSequence;
//@doc:inline
fragment SingleQuotedUnicodeStringCharacter: ~['\r\n\\] | EscapeSequence;

// 注意，这也将用于Yul十六进制字符串字面量。
/**
 * 十六进制字符串需要包含偶数个十六进制数字，可以使用下划线分组。
 */
HexString: 'hex' (('"' EvenHexDigits? '"') | ('\'' EvenHexDigits? '\''));
/**
 * 十六进制数字由前缀和可以用下划线分隔的任意数量的十六进制数字组成。
 */
HexNumber: '0' 'x' HexDigits;
//@doc:inline
fragment HexDigits: HexCharacter ('_'? HexCharacter)*;
//@doc:inline
fragment EvenHexDigits: HexCharacter HexCharacter ('_'? HexCharacter HexCharacter)*;
//@doc:inline
fragment HexCharacter: [0-9A-Fa-f];

/**
 * 一个十进制数字的字面量由十进制数字组成，可以用下划线和一个可选的正负指数来分隔。
 * 如果这些数字包含一个小数点，则该数字具有定点类型。
 */
DecimalNumber: (DecimalDigits | (DecimalDigits? '.' DecimalDigits)) ([eE] '-'? DecimalDigits)?;
//@doc:inline
fragment DecimalDigits: [0-9] ('_'? [0-9])* ;


/**
 * solidity中的标识符必须以字母，美元符号或下划线开头，并且可以在第一个符号之后再包含数字。
 */
Identifier: IdentifierStart IdentifierPart*;
//@doc:inline
fragment IdentifierStart: [a-zA-Z$_];
//@doc:inline
fragment IdentifierPart: [a-zA-Z0-9$_];

WS: [ \t\r\n\u000C]+ -> skip ;
COMMENT: '/*' .*? '*/' -> channel(HIDDEN) ;
LINE_COMMENT: '//' ~[\r\n]* -> channel(HIDDEN);

mode AssemblyBlockMode;

//@doc:inline
AssemblyDialect: '"evmasm"';
AssemblyLBrace: '{' -> popMode, pushMode(YulMode);

AssemblyBlockWS: [ \t\r\n\u000C]+ -> skip ;
AssemblyBlockCOMMENT: '/*' .*? '*/' -> channel(HIDDEN) ;
AssemblyBlockLINE_COMMENT: '//' ~[\r\n]* -> channel(HIDDEN) ;

mode YulMode;

YulBreak: 'break';
YulCase: 'case';
YulContinue: 'continue';
YulDefault: 'default';
YulFalse: 'false';
YulFor: 'for';
YulFunction: 'function';
YulIf: 'if';
YulLeave: 'leave';
YulLet: 'let';
YulSwitch: 'switch';
YulTrue: 'true';
YulHex: 'hex';

/**
 * EVM Yul语言的内置函数。
 */
YulEVMBuiltin:
	'stop' | 'add' | 'sub' | 'mul' | 'div' | 'sdiv' | 'mod' | 'smod' | 'exp' | 'not'
	| 'lt' | 'gt' | 'slt' | 'sgt' | 'eq' | 'iszero' | 'and' | 'or' | 'xor' | 'byte'
	| 'shl' | 'shr' | 'sar' | 'addmod' | 'mulmod' | 'signextend' | 'keccak256'
	| 'pop' | 'mload' | 'mstore' | 'mstore8' | 'sload' | 'sstore' | 'msize' | 'gas'
	| 'address' | 'balance' | 'selfbalance' | 'caller' | 'callvalue' | 'calldataload'
	| 'calldatasize' | 'calldatacopy' | 'extcodesize' | 'extcodecopy' | 'returndatasize'
	| 'returndatacopy' | 'extcodehash' | 'create' | 'create2' | 'call' | 'callcode'
	| 'delegatecall' | 'staticcall' | 'return' | 'revert' | 'selfdestruct' | 'invalid'
	| 'log0' | 'log1' | 'log2' | 'log3' | 'log4' | 'chainid' | 'origin' | 'gasprice'
	| 'blockhash' | 'coinbase' | 'timestamp' | 'number' | 'difficulty' | 'gaslimit'
	| 'basefee';

YulLBrace: '{' -> pushMode(YulMode);
YulRBrace: '}' -> popMode;
YulLParen: '(';
YulRParen: ')';
YulAssign: ':=';
YulPeriod: '.';
YulComma: ',';
YulArrow: '->';

/**
 * Yul标识符由字母，美元符号，下划线和数字组成，但不能以数字开头。
 * 在内联程序中，用户定义的标识符中不能有圆点。相反，对于由带点的标识符组成的表达式，请参阅yulPath。
 */
YulIdentifier: YulIdentifierStart YulIdentifierPart*;
//@doc:inline
fragment YulIdentifierStart: [a-zA-Z$_];
//@doc:inline
fragment YulIdentifierPart: [a-zA-Z0-9$_];
/**
 * Yul中的十六进制字由一个前缀和一个或多个十六进制数字组成。
 */
YulHexNumber: '0' 'x' [0-9a-fA-F]+;
/**
 * Yul中的小数字面量可以是零或任何不含前导零的小数位序列。
 */
YulDecimalNumber: '0' | ([1-9] [0-9]*);
/**
 * Yul中的字符串字面量由一个或多个双引号或单引号字符串组成，
 * 这些字符串可能包含转义序列和可打印字符
 * 未转义的换行符或未转义的双引号或单引号除外。
 */
YulStringLiteral:
	'"' DoubleQuotedStringCharacter* '"'
	| '\'' SingleQuotedStringCharacter* '\'';
//@doc:inline
YulHexStringLiteral: HexString;

YulWS: [ \t\r\n\u000C]+ -> skip ;
YulCOMMENT: '/*' .*? '*/' -> channel(HIDDEN) ;
YulLINE_COMMENT: '//' ~[\r\n]* -> channel(HIDDEN) ;

mode PragmaMode;

/**
 * 编译指示令牌。可以包含除分号以外的任何类型的符号。
 * 注意，目前solidity解析器只允许它的一个子集。
 */
//@doc:name pragma-token
//@doc:no-diagram
PragmaToken: ~[;]+;
PragmaSemicolon: ';' -> popMode;

PragmaWS: [ \t\r\n\u000C]+ -> skip ;
PragmaCOMMENT: '/*' .*? '*/' -> channel(HIDDEN) ;
PragmaLINE_COMMENT: '//' ~[\r\n]* -> channel(HIDDEN) ;
