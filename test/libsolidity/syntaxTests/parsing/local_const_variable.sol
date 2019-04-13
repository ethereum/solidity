contract Foo {
	function localConst() returns (uint ret)
	{
		uint constant local = 4;
		return local;
	}
}
// ----
// ParserError: (85-86): Expected ';' but got 'constant'; deleted tokens to the next expected token.
