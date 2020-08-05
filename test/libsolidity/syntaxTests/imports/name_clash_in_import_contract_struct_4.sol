==== Source: a ====
contract A {}
==== Source: b ====
import {A} from "a";
struct A { uint256 a; }
// ----
// DeclarationError 2333: (b:21-44): Identifier already declared.
