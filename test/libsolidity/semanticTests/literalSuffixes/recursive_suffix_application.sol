function double(uint x) pure suffix returns (uint) {
    if (x == 0)
        return 0;
    if (x == 1)
        return 0 double + 2;
    else
        return 1 double * x;
}

contract C {
    function test() public pure returns (uint) {
        return 10 double;
    }
}
// ----
// test() -> 20
