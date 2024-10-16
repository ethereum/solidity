// Tests that private functions are not overridden by inheriting contracts.
contract A {
	function foo() private {}
}
contract B is A {
	function foo() private override {}
}
// ----
// TypeError 7792: (160-168): Function has override specified but does not override anything.
