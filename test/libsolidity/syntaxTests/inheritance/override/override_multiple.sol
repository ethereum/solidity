contract A {
	function foo() internal returns (uint256);
}
contract X {
	int public override testvar;
	function test() internal override returns (uint256);
	function foo() internal override(X, A) returns (uint256);
}
// ----
