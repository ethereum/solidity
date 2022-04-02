contract C {
	uint s;
	function f() public constant returns (uint) {
		return s;
	}
}
// ----
// ParserError 2314: (43-51='constant'): Expected '{' but got 'constant'
