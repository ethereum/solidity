contract Foo {
	uint[] memory storage calldata x;
}
// ----
// ParserError 2314: (23-29='memory'): Expected identifier but got 'memory'
