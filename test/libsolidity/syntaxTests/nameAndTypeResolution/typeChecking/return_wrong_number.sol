contract test {
    function f() public returns (bool r1, bool r2) { return 1 >= 2; }
}
// ----
// TypeError 8863: (69-82='return 1 >= 2'): Different number of arguments in return statement than in returns declaration.
