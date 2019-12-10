==== Source: a ====
struct A { uint256 a; }
==== Source: b ====
import {A} from "a";
contract A {}
// ----
// DeclarationError: (b:21-34): Identifier already declared.
