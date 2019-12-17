contract A {
	function foo() external virtual pure returns(uint) { return 5; }
}
contract B is A {
	function foo() external virtual override pure returns(uint) { return 5; }
}
contract C is A {
	function foo() external virtual override pure returns(uint) { return 5; }
}
contract X is B, C {
	uint public override foo;
}
// ----
// TypeError: (305-313): Public state variable needs to specify overridden contracts "B" and "C".
