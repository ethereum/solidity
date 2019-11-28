contract A {
	function foo() external virtual pure returns(uint) { return 4; }
	function foo(uint ) external virtual pure returns(uint) { return 4; }
	function foo(uint , uint ) external pure virtual returns(A) {  }
}
contract X is A {
	uint public override foo;
}
// ----
