contract C {
    function f() pure public { assert; }
}
// ----
// Warning 6133: (44-50='assert'): Statement has no effect.
