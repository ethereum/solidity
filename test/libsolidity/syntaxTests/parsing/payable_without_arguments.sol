contract C {
	function f() public pure {
		address payable q = payable;
	}
}
// ----
// ParserError 2314: (70-71): Expected '(' but got ';'
