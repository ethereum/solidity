contract C {
    function f() pure public { selfdestruct; }
}
// ----
// Warning 6133: (44-56): Statement has no effect.
