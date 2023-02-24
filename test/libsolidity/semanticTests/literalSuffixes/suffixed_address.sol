function not(address a) pure suffix returns (bytes20) {
    return ~bytes20(a);
}

contract C {
    function notMin() public pure returns (bytes20) {
        return 0x0000000000000000000000000000000000000000 not;
    }

    function notMax() public pure returns (bytes20) {
        return 0xFFfFfFffFFfffFFfFFfFFFFFffFFFffffFfFFFfF not;
    }
}
// ----
// notMin() -> 0xffffffffffffffffffffffffffffffffffffffff000000000000000000000000
// notMax() -> 0x0000000000000000000000000000000000000000
