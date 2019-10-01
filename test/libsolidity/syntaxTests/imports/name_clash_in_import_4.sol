==== Source: a ====
contract A {}
==== Source: b ====
import {A} from "a"; contract A {}
// ----
// DeclarationError: (b:21-34): Identifier already declared.
