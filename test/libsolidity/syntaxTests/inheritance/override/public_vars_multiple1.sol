contract A {
	uint public foo;
}
contract B {
	function foo() external virtual view returns(uint) { return 5; }
}
contract X is A, B {
	uint public override foo;
}
// ----
// DeclarationError 9097: (136-160): Identifier already declared.
// TypeError 1452: (14-29): Cannot override public state variable.
// TypeError 4327: (148-156): Public state variable needs to specify overridden contracts "A" and "B".
