contract C {
    function f() pure public {
        string("abc");
    }
}
// ----
// Warning 6133: (52-65): Statement has no effect.
