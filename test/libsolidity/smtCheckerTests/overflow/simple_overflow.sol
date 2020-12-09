pragma experimental SMTChecker;
contract C {
    function f(uint a, uint b) public pure returns (uint) { return a + b; }
}
// ----
// Warning 4984: (112-117): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\n\na = 1\nb = 115792089237316195423570985008687907853269984665640564039457584007913129639935\n = 0\n\nTransaction trace:\nconstructor()\nf(1, 115792089237316195423570985008687907853269984665640564039457584007913129639935)
