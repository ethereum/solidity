contract A {
	function foo() external virtual view returns(uint) { return 5; }
}
contract B is A {
	uint public override foo;
}
contract C is A {
	function foo() external virtual override view returns(uint) { return 5; }
}
contract X is B, C {
	uint public override(A, C) foo;
}
// ----
// DeclarationError 9097: (245-275): Identifier already declared.
// TypeError 1452: (100-124): Cannot override public state variable.
// TypeError 4327: (257-271): Public state variable needs to specify overridden contract "B".
// TypeError 2353: (257-271): Invalid contract specified in override list: "A".
