==== Source: a ====
contract A {}
==== Source: b ====
import {A as b} from "a"; contract b {}
// ----
// DeclarationError: (b:26-39): Identifier already declared.
