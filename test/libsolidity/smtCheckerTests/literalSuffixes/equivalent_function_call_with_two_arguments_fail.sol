function suffix(uint m, uint e) pure suffix returns (uint) {
    return m + e;
}

contract C {
    function test() public pure {
        assert(1.23 suffix == suffix(123, 4));
        assert(123 suffix == suffix(12, 3));
        assert(0 suffix == suffix(0, 1));
    }
}

// ====
// SMTEngine: all
// ----
// Warning 6328: (137-174): CHC: Assertion violation happens here.
// Warning 6328: (184-219): CHC: Assertion violation happens here.
// Warning 6328: (229-261): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
