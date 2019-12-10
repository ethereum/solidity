contract A {
	function f() external virtual {}
}
contract B {
	function f() external virtual {}
}
contract C is A, B {
	function f() external override (A, B) {}
}
contract X is C {
}
// ----
