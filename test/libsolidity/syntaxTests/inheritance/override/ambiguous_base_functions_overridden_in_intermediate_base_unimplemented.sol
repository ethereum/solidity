contract A {
	function f() external virtual {}
}
contract B {
	function f() external virtual {}
}
contract C is A, B {
	function f() external override (A, B);
}
contract X is C {
}
// ----
// TypeError: (120-158): Overriding an implemented function with an unimplemented function is not allowed.
// TypeError: (120-158): Overriding an implemented function with an unimplemented function is not allowed.
// TypeError: (120-158): Functions without implementation must be marked virtual.
