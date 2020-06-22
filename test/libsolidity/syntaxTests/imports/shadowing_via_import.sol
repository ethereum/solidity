==== Source: a ====
library A {}
==== Source: b ====
library A {}
==== Source: c ====
import {A} from "./a"; import {A} from "./b";
// ----
// DeclarationError 2333: (c:31-32): Identifier already declared.
