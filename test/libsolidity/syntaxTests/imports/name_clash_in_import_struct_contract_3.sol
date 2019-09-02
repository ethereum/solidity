==== Source: a ====
struct A { uint256 a; }
==== Source: b ====
import {A as b} from "a";
contract b {}
// ----
// DeclarationError: (b:26-39): Identifier already declared.
