contract C {
    function f() pure public {
        1 revert;
    }
}
// ----
// TypeError 9322: (52-60): No matching declaration found after argument-dependent lookup.
