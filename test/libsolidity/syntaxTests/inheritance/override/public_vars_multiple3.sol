contract A {
	function foo() external virtual pure returns(A) {  }
	function foo(uint ) external virtual pure returns(uint) { return 4; }
	function foo(uint , uint ) external pure virtual returns(A) {  }
}
contract X is A {
	uint public override foo;
}
// ----
// TypeError: (225-249): Overriding public state variable return types differ.
