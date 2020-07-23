contract A {
	function foo() external virtual view returns(uint) { return 5; }
}
contract B {
	function foo() external virtual view returns(uint) { return 5; }
}
contract X is A, B {
	uint public override foo;
}
// ----
// TypeError 4327: (196-204): Public state variable needs to specify overridden contracts "A" and "B".
