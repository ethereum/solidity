contract C {
    function f() pure public {
        ((2**270) / 2**100, 1);
    }
}
// ----
// Warning: (52-74): Statement has no effect.
