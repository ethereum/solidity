contract C {
    function f() pure public { selfdestruct; }
}
// ----
// Warning: (44-56): Statement has no effect.
