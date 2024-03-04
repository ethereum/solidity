contract C {
    uint immutable x = 0;
    uint y = f();

    function f() internal pure returns(uint) { return x; }
}
// ----
