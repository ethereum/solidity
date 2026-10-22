// Tests that private functions do not shadow, and are not shadowed by, inheriting contract's functions.
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
