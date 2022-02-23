==== Source: a ====
enum E { A }
==== Source: b ====
import "a";
enum E { A }
// ----
// DeclarationError 2333: (b:12-24): Identifier already declared.
