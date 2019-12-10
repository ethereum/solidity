==== Source: a ====
contract A {}
==== Source: b ====
import {A as b} from "a";
struct b { uint256 a; }
// ----
// DeclarationError: (b:26-49): Identifier already declared.
