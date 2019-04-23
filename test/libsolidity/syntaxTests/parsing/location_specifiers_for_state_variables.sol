contract Foo {
	uint[] memory x;
}
// ----
// ParserError: (23-29): Expected identifier but got 'memory'
// ParserError: (30-31): Expected ';' but got identifier
// ParserError: (31-32): Expected identifier but got ';'
// ParserError: (33-34): Expected ';' but got '}'
