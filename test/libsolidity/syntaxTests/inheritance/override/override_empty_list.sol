contract A {
	int public testvar;
	function test() internal returns (uint256);
}
contract X is A {
	int public override testvar;
	function test() internal override() returns (uint256);
}
// ----
// ParserError: (164-165): Expected identifier but got ')'
