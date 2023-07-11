contract C {
    function f() public pure {
        uint v = 1;
        v &= 1;
        assert(v == 1);

        v = 7;
        v &= 3;
        assert(v != 3); // fails, as 7 & 3 = 3

        uint c = 0;
        c &= v;
        assert(c == 0);

        uint8 x = 0xff;
        uint16 y = 0xffff;
        y &= x;
        assert(y == 0xff);
        assert(y == 0xffff); // fails

        y = 0xffff;
        x = 0xff;
        y &= y | x;
        assert(y == 0xffff);
        assert(y == 0xff); // fails
    }
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (144-158): CHC: Assertion violation happens here.
// Warning 6328: (347-366): CHC: Assertion violation happens here.
// Warning 6328: (473-490): CHC: Assertion violation happens here.
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
