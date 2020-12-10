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
// Warning 6321: (83-87): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6328: (204-221): CHC: Assertion violation happens here.\nCounterexample:\n\n\n = 0\n\nTransaction trace:\nconstructor()\nf()
