==== Source: a ====
enum E { A }
==== Source: b ====
import "a";
enum E { A }
// ----
// DeclarationError: (b:12-24): Identifier already declared.
