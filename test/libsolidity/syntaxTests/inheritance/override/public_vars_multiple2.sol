contract A {
	function foo() external virtual view returns(uint) { return 4; }
	function foo(uint ) external virtual view returns(uint) { return 4; }
	function foo(uint , uint ) external view virtual returns(A) {  }
}
contract X is A {
	uint public override foo;
}
// ----
