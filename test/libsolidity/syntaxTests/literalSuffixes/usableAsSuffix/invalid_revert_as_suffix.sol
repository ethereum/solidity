contract C {
    function f() pure public {
        1 revert;
    }
}
// ----
// TypeError 2144: (52-60): No matching declaration found after variable lookup.
