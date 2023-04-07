contract C {
    function f() pure public {
        1 revert;
    }
}
// ----
// TypeError 2144: (54-60): No matching declaration found after variable lookup.
