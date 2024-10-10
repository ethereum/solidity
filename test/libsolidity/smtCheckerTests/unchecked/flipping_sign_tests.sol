contract test {
    function f() public pure returns (bool) {
        int256 x = -2**255;
        unchecked { assert(-x == x); }
        assert(-x == x);
        return true;
    }
}
// ====
// SMTEngine: all
// ----
// Warning 4984: (117-119): CHC: Overflow (resulting value larger than 2**255 - 1) happens here.
// Warning 6328: (110-125): CHC: Assertion violation happens here.
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
