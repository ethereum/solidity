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
// TypeError: (162-211): Derived contract must override function "foo". Two or more base classes define function with same name and parameter types.
