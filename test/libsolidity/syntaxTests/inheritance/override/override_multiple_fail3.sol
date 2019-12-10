contract A {
	function foo() internal returns (uint256);
}
contract X {
	function foo() internal override(X, address) returns (uint256);
}
// ----
// ParserError: (109-116): Expected identifier but got 'address'
