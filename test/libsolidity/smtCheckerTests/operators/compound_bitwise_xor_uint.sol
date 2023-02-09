contract C {
    function f() public pure {
        uint v = 7;
        v ^= 3;
        assert(v != 4); // fails, as 7 ^ 3 = 4

        uint c = 0;
        c ^= v;
        assert(c == 4);

        uint16 x = 0xff;
        uint16 y = 0xffff;
        y ^= x;
        assert(y == 0xff); // fails
        assert(y == 0xff00);

        y = 0xf1;
        x = 0xff00;
        y ^= x | y;
        assert(y == 0xffff); // fails
        assert(x == 0xff00);
    }
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (88-102): CHC: Assertion violation happens here.
// Warning 6328: (265-282): CHC: Assertion violation happens here.
// Warning 6328: (389-408): CHC: Assertion violation happens here.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
