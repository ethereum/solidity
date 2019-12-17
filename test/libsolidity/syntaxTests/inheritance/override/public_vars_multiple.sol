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
// TypeError: (196-204): Public state variable needs to specify overridden contracts "A" and "B".
