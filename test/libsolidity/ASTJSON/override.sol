contract A {
	function faa() public {}
}
contract B is A {
	function foo() public;
	function faa() public override {}
}
contract C is B {
	function foo() public override { }
	function faa() public override(A, B) {}
}

// ----
