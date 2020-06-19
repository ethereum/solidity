==== Source: a ====
struct A { uint256 a; }
==== Source: b ====
import "a" as A;
struct A { uint256 a; }
// ----
// DeclarationError 2333: (b:17-40): Identifier already declared.
