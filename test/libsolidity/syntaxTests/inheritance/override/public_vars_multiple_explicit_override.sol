contract A {
	function foo() external virtual pure returns(uint) { return 5; }
}
contract B {
	function foo() external virtual pure returns(uint) { return 5; }
}
contract X is A, B {
	uint public override(A, B) foo;
}
// ----
// TypeError: (162-217): Derived contract must override function "foo". Two or more base classes define function with same name and parameter types.
