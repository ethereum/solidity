==== Source: a ====
contract A {}
==== Source: b ====
import "a"; contract A {}
// ----
// DeclarationError: (b:12-25): Identifier already declared.
