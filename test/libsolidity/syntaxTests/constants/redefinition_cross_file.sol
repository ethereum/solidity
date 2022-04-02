==== Source: a ====
import "b";
uint constant c = 7;
==== Source: b ====
import "a";
uint constant c = 7;
// ----
// DeclarationError 2333: (b:12-31='uint constant c = 7'): Identifier already declared.
