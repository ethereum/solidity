==== Source: a ====
contract A { }
==== Source: b ====
import {C} from "a";
contract B { }
// ----
// DeclarationError 2904: (b:0-20): Declaration "C" not found in "a" (referenced as "a").
