contract C {
    function f() pure public {
        uint a;
    }
}
// ----
// Warning 2072: (52-58='uint a'): Unused local variable.
