contract C {
    function f() pure public {
        string("abc");
    }
}
// ----
// Warning: (52-65): Statement has no effect.
