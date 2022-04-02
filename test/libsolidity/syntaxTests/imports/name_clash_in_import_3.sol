==== Source: a ====
contract A {}
==== Source: b ====
import {A as b} from "a"; contract b {}
// ----
// DeclarationError 2333: (b:26-39='contract b {}'): Identifier already declared.
