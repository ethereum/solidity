contract C {
    function f(uint a, uint b) public pure returns (uint) { return a + b; }
}
// ====
// SMTEngine: all
// ----
// Warning 4984: (80-85): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\n\na = 1\nb = 115792089237316195423570985008687907853269984665640564039457584007913129639935\n = 0\n\nTransaction trace:\nC.constructor()\nC.f(1, 115792089237316195423570985008687907853269984665640564039457584007913129639935)
