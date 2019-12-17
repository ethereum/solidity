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
// TypeError: (100-124): Cannot override public state variable.
// TypeError: (257-265): Public state variable needs to specify overridden contracts "B" and "C".
