contract test {
    function f() pure public { 0.x; }
}
// ----
// TypeError 9582: (47-50='0.x'): Member "x" not found or not visible after argument-dependent lookup in int_const 0.
