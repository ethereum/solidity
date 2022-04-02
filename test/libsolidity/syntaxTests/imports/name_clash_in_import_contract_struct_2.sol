==== Source: a ====
contract A {}
==== Source: b ====
import "a" as A;
struct A { uint256 a; }
// ----
// DeclarationError 2333: (b:17-40='struct A { uint256 a; }'): Identifier already declared.
