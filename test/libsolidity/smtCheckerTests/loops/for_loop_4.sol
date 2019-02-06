pragma experimental SMTChecker;
contract C {
    function f(uint x) public pure {
        for (uint y = 2; x < 10; y = 3) {
            assert(y == 2);
        }
    }
}
// ----
// Warning: (136-150): Assertion violation happens here
// Warning: (115-120): Underflow (resulting value less than 0) happens here
// Warning: (115-120): Overflow (resulting value larger than 2**256 - 1) happens here
