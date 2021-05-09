contract C {
    function f() public returns (uint256) {
        uint256[][] memory x = new uint256[][](42);
        assert(x[0].length == 0);
        x[0] = new uint256[](1);
        x[0][0] = 1;
        assert(x[4].length == 0);
        x[4] = new uint256[](1);
        x[4][0] = 2;
        assert(x[10].length == 0);
        x[10] = new uint256[](1);
        x[10][0] = 44;
        uint256[][] memory y = new uint256[][](24);
        assert(y[0].length == 0);
        y[0] = new uint256[](1);
        y[0][0] = 1;
        assert(y[4].length == 0);
        y[4] = new uint256[](1);
        y[4][0] = 2;
        assert(y[10].length == 0);
        y[10] = new uint256[](1);
        y[10][0] = 88;
        if (
            (x[0][0] == y[0][0]) &&
            (x[4][0] == y[4][0]) &&
            (x[10][0] == 44) &&
            (y[10][0] == 88)
        ) return 7;
        return 0;
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f() -> 7
