pragma experimental SMTChecker;
contract C {
    function f() public pure returns (byte) {
        byte a = 0xff;
        byte b = 0xf0;
        a ^= ~b;
        assert(a == b);

        a ^= ~b;
        assert(a != 0xff); // fails
    }
}
// ----
// Warning 6328: (204-221): CHC: Assertion violation happens here.
