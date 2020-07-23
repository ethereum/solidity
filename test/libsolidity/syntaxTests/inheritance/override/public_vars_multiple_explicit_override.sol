contract A {
	function foo() external virtual view returns(uint) { return 5; }
}
contract B {
	function foo() external virtual view returns(uint) { return 5; }
}
contract X is A, B {
	uint public override(A, B) foo;
}
// ----
