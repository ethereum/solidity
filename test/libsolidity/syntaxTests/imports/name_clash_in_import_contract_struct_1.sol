==== Source: a ====
contract A {}
==== Source: b ====
import "a";
struct A { uint256 a; }
// ----
// DeclarationError: (b:12-35): Identifier already declared.
