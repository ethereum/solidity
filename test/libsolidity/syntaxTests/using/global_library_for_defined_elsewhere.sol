using L for L.S global;
library L {
    struct S { uint x; }
}
// ----
// TypeError 4117: (0-23): Can only use "global" with types defined in the same source unit at file level.
