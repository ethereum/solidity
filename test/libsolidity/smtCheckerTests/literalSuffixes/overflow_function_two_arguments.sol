function overflowSuffix(uint8 m, uint8 e) pure suffix returns (uint) {
    return m + e;
}

function underflowSuffix(uint8 m, uint8 e) pure suffix returns (uint8) {
    return m - e;
}

function constantSuffix(uint8, uint8) pure suffix returns (uint8) {
    return 1;
}

contract C {
    function overflow() public pure {
        25.5 overflowSuffix;
    }

    function underflow() public pure {
        0.01 underflowSuffix;
    }

    function notUnderOverflow() public pure {
        25.5 constantSuffix;
        0.01 constantSuffix;
    }
}
// ====
// SMTEngine: all
// ----
// Warning 4984: (82-87): CHC: Overflow (resulting value larger than 255) happens here.
// Warning 3944: (176-181): CHC: Underflow (resulting value less than 0) happens here.
