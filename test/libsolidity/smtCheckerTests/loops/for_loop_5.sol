pragma experimental SMTChecker;
contract C {
    function f(uint x) public pure {
        uint y;
        for (y = 2; x < 10; ) {
            y = 3;
        }
        assert(y == 3);
    }
}
// ----
// Warning: (167-181): Assertion violation happens here
// Warning: (142-147): Underflow (resulting value less than 0) happens here
// Warning: (142-147): Overflow (resulting value larger than 2**256 - 1) happens here
