function isNegative1(int x) pure suffix returns (bool) {
    return x < 0;
}

function isNegative2(int mantissa, uint) pure suffix returns (bool) {
    return mantissa < 0;
}

contract C {
    function run() public pure returns (bool, bool) {
        return (isNegative1(-1), isNegative2(-1, 0));
    }
}
// ----
// run() -> true, true
