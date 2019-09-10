contract test {
    function f() public returns (bool r1, bool r2) { return 1 >= 2; }
}
// ----
// TypeError: (69-82): Different number of arguments in return statement than in returns declaration.
