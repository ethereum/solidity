contract Foo {
	function localConst() returns (uint ret)
	{
		uint constant local = 4;
		return local;
	}
}
// ----
// ParserError: (67-75): Expected ';' but got 'constant'
