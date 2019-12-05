contract A {
	function foo() external virtual pure returns(uint) { return 5; }
}
contract B {
	function foo() external virtual pure returns(uint) { return 5; }
}
contract X is A, B {
	uint public override foo;
}
// ----
// TypeError: (162-211): Derived contract must override function "foo". Function with the same name and parameter types defined in two or more base classes.
