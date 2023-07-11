==== Source: A ====
contract Foo {
	uint[] memory storage calldata x;
}
==== Source: B ====
contract Foo {
	uint[] storage storage x;
}
// ----
// ParserError 2314: (A:23-29): Expected identifier but got 'memory'
// ParserError 2314: (B:23-30): Expected identifier but got 'storage'