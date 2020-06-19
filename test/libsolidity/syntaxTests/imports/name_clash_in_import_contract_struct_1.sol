==== Source: a ====
contract A {}
==== Source: b ====
import "a";
struct A { uint256 a; }
// ----
// DeclarationError 2333: (b:12-35): Identifier already declared.
