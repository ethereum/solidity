contract C {
    function f(uint x) pure public {
        if (x > 7)
            revert;
    }
}
// ----
// Warning 6133: (81-87): Statement has no effect.
