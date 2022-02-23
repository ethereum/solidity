contract C {
    error E();
    uint x = 2;
    function f() public {
        revert E();
        x = 4;
    }
}
// ----
// Warning 5740: (98-103): Unreachable code.
