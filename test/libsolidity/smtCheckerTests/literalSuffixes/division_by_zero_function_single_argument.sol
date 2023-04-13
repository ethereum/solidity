function suffix(uint x) pure suffix returns (uint) { return 1000000 / x; }
contract C {
    uint y = 0 suffix;
}
// ====
// SMTEngine: all
// ----
// Warning 4281: (60-71): CHC: Division by zero happens here.
