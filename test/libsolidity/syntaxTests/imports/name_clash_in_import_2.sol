==== Source: a ====
contract A {}
==== Source: b ====
import "a" as A; contract A {}
// ----
// DeclarationError 2333: (b:17-30): Identifier already declared.
