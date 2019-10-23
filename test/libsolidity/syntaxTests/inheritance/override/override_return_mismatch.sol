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
// TypeError: (0-79): Contract "A" should be marked as abstract.
// TypeError: (80-181): Contract "B" should be marked as abstract.
// TypeError: (182-288): Derived contract must override function "foo". Function with the same name and parameter types defined in two or more base classes.
// TypeError: (182-288): Contract "X" should be marked as abstract.
