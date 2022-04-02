using {f} for L.S global;
function f(L.S memory) pure{}
library L {
    struct S { uint x; }
}
// ----
// TypeError 4117: (0-25='using {f} for L.S global;'): Can only use "global" with types defined in the same source unit at file level.
