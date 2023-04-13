function suffix(uint m, uint e) pure suffix returns (uint) {
    return m + e;
}

contract C {
    function test() public pure {
        assert(1.23 suffix == suffix(123, 2));
        assert(123 suffix == suffix(123, 0));
        assert(0 suffix == suffix(0, 0));
    }
}

// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
