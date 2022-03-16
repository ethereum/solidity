==== Source: A ====
struct S { uint x; }
==== Source: B ====

using {f} for S global;
using {f} for A.S global;

function f(S memory) pure{}

import {S} from "A";
import "A" as A;
// ----
// TypeError 4117: (B:1-24): Can only use "global" with types defined in the same source unit at file level.
// TypeError 4117: (B:25-50): Can only use "global" with types defined in the same source unit at file level.
