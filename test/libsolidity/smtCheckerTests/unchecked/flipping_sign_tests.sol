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
// Warning 6328: (110-125): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
