function suffix2(uint m, uint e) pure suffix returns (uint) {
    return m / e;
}

contract C {
    function test() public pure returns (uint x) {
        x = 1234567890 suffix2;
    }
}

// ====
// SMTEngine: all
// ----
// Warning 4281: (73-78): CHC: Division by zero happens here.
