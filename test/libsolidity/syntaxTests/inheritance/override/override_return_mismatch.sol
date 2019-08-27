contract A {
	int public testvar;
	function foo() internal returns (uint256);
}
contract B {
	function foo() internal returns (uint8);
	function test() internal returns (uint256);
}
contract X is A, B {
	int public override testvar;
	function test() internal override returns (uint256);
}
// ----
// TypeError: (94-134): Functions from multiple base classes conflict: return types differ.
