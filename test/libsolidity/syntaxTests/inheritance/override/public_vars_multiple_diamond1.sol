contract A {
	function foo() external virtual pure returns(uint) { return 5; }
}
contract B is A {
	uint public override foo;
}
contract C is A {
	function foo() external virtual override pure returns(uint) { return 5; }
}
contract X is B, C {
	uint public override foo;
}
// ----
// DeclarationError: (245-269): Identifier already declared.
// TypeError: (223-272): Derived contract must override function "foo". Function with the same name and parameter types defined in two or more base classes.
