contract test {
    function f() pure public { 1.x; }
}
// ----
// TypeError 9582: (47-50): Member "x" not found or not visible after argument-dependent lookup in int_const 1.
