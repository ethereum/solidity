abstract contract A {
	int public testvar;
	function test() internal virtual returns (uint256);
	function test2() internal virtual returns (uint256);
}
contract X is A {
	int public override testvar;
	function test() internal override returns (uint256) {}
	function test2() internal override(A) returns (uint256) {}
}
// ----
// DeclarationError: (171-198): Identifier already declared.
