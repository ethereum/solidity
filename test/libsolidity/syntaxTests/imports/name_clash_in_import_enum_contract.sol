==== Source: a ====
enum E { A }
==== Source: b ====
import "a";
contract E { }
// ----
// DeclarationError 2333: (b:12-26='contract E { }'): Identifier already declared.
