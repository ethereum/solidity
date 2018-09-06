contract A {
    function f() public pure {
        uint y = 1;
        uint x = 3 < 0 ? y = 3 : 6;
        true ? x = 3 : 4;
    }
}
// ----
