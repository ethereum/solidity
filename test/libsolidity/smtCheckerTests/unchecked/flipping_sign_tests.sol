contract test {
    function f() public pure returns (bool) {
        int256 x = -2**255;
        unchecked { assert(-x == x); }
        assert(-x == x); // CHC apparently does not create an underflow target for unary minus
        return true;
    }
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (110-125): CHC: Assertion violation happens here.\nCounterexample:\n\n = false\nx = (- 57896044618658097711785492504343953926634992332820282019728792003956564819968)\n\nTransaction trace:\ntest.constructor()\ntest.f()
