pragma experimental SMTChecker;

contract C {
    uint a;
    bool b;

    function f() public returns(uint c) {
        c = b ? a + 1 : a--;
        assert(c > a);
    }
}
// ----
// Warning 4984: (129-134): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\na = 115792089237316195423570985008687907853269984665640564039457584007913129639935, b = false\n\nc = 0\n\nTransaction trace:\nconstructor()\nState: a = 0, b = false\nf()\nState: a = 115792089237316195423570985008687907853269984665640564039457584007913129639935, b = false\nf()
// Warning 3944: (137-140): CHC: Underflow (resulting value less than 0) happens here.\nCounterexample:\na = 0, b = false\n\nc = 0\n\nTransaction trace:\nconstructor()\nState: a = 0, b = false\nf()
// Warning 6328: (150-163): CHC: Assertion violation happens here.\nCounterexample:\na = 115792089237316195423570985008687907853269984665640564039457584007913129639935, b = false\n\nc = 0\n\nTransaction trace:\nconstructor()\nState: a = 0, b = false\nf()
