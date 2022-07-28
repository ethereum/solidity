contract A {
	function foo() private {}
}
contract B is A {
	function foo() private {}
}
contract C is B {
	function foo() public {}
}
// ----
