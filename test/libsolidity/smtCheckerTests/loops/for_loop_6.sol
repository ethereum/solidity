pragma experimental SMTChecker;
contract C {
    function f(uint x) public pure {
        uint y;
        for (y = 2; x < 10; ) {
            y = 3;
        }
        // False positive due to resetting y.
        assert(y < 4);
    }
}
// ----
// Warning: (213-226): Assertion violation happens here
// Warning: (142-147): Underflow (resulting value less than 0) happens here
// Warning: (142-147): Overflow (resulting value larger than 2**256 - 1) happens here
