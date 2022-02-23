==== Source: a ====
contract C { D d; }
==== Source: b ====
import "a"; contract D is C {}
// ----
// DeclarationError 7920: (a:13-14): Identifier not found or not unique.
