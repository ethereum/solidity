==== Source: a ====
struct A { uint256 a; }
==== Source: b ====
import "a" as A;
contract A {}
// ----
// DeclarationError: (b:17-30): Identifier already declared.
