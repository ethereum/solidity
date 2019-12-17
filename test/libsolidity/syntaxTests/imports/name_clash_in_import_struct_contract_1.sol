==== Source: a ====
struct A { uint256 a; }
==== Source: b ====
import "a";
contract A {}
// ----
// DeclarationError: (b:12-25): Identifier already declared.
