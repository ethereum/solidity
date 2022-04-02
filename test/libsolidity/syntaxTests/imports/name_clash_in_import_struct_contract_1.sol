==== Source: a ====
struct A { uint256 a; }
==== Source: b ====
import "a";
contract A {}
// ----
// DeclarationError 2333: (b:12-25='contract A {}'): Identifier already declared.
