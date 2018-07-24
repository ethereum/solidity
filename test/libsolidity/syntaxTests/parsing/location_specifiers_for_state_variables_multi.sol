contract Foo {
	uint[] memory storage calldata x;
}
// ----
// ParserError: (23-29): Expected identifier but got 'memory'
