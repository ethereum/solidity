==== Source: a ====
enum E { A }
==== Source: b ====
import "a";
struct E { uint256 a; }
// ----
// DeclarationError: (b:12-35): Identifier already declared.
