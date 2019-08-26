contract A {
	function foo() internal returns (uint256);
}
contract X {
	int public override(A,) testvar;
}
// ----
// ParserError: (95-96): Expected identifier but got ')'
