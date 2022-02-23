==== Source: a ====
struct A { uint256 a; }
==== Source: b ====
import {A as b} from "a";
contract b {}
// ----
// DeclarationError 2333: (b:26-39): Identifier already declared.
