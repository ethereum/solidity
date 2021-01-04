pragma experimental SMTChecker;

contract test {
    function f() public pure returns (bool) {
        int256 x = -2**255;
        unchecked { assert(-x == x); }
        assert(-x == x); // CHC apparently does not create an underflow target for unary minus
        return true;
    }
}
// ----
// Warning 6328: (143-158): CHC: Assertion violation happens here.\nCounterexample:\n\n\n = false\n\nTransaction trace:\ntest.constructor()\ntest.f()
// Warning 2661: (150-152): BMC: Overflow (resulting value larger than 0x80 * 2**248 - 1) happens here.
