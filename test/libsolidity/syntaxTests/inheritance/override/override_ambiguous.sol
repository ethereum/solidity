contract A {
	int public testvar;
	function foo() internal returns (uint256);
}
contract B {
	function foo() internal returns (uint256);
	function test() internal returns (uint256);
}
contract X is A, B {
	int public override testvar;
	function test() internal override returns (uint256);
}
// ----
// TypeError: (184-290): Derived contract must override function "foo". Function with the same name and parameter types defined in two or more base classes.
