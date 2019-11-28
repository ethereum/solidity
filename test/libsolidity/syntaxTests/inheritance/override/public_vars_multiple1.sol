contract A {
	uint public foo;
}
contract B {
	function foo() external virtual pure returns(uint) { return 5; }
}
contract X is A, B {
	uint public override foo;
}
// ----
// DeclarationError: (136-160): Identifier already declared.
