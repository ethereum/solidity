==== Source: A ====
contract A {
	function g() public { x; }
}
==== Source: B ====
contract B {
	function f() public { }
}
// ----
// DeclarationError 7576: (A:36-37): Undeclared identifier.
