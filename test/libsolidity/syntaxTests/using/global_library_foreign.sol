==== Source: A ====
struct S { uint x; }
==== Source: B ====
library L {}
using L for S global;
import {S} from "A";
// ----
// TypeError 4117: (B:13-34): Can only use "global" with types defined in the same source unit at file level.
