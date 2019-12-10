==== Source: a ====
enum E { A }
==== Source: b ====
import "a";
contract E { }
// ----
// DeclarationError: (b:12-26): Identifier already declared.
