==== Source: a ====
contract A {}
==== Source: b ====
import {A} from "a"; contract A {}
// ----
// DeclarationError 2333: (b:21-34='contract A {}'): Identifier already declared.
