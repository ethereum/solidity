==== Source: A ====
contract A {
	function g() public { x; }
}
==== Source: B ====
contract B {
	function f() public { }
}
// ----
// DeclarationError: (A:36-37): Undeclared identifier.
