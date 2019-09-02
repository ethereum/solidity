==== Source: a ====
contract A {}
==== Source: b ====
import "a" as A;
struct A { uint256 a; }
// ----
// DeclarationError: (b:17-40): Identifier already declared.
