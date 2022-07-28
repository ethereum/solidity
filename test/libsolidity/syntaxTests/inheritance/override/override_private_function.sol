contract A {
	function foo() private {}
}
contract B is A {
	function foo() private override {}
}
// ----
// TypeError 7792: (84-92): Function has override specified but does not override anything.
