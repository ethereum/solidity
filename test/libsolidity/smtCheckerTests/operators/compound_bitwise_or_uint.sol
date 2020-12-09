pragma experimental SMTChecker;

contract C {
    function f() public pure {
        uint v = 7;
        v |= 3;
        assert(v != 7); // fails, as 7 | 3 = 7

        uint c = 0;
        c |= v;
        assert(c == 7);

        uint16 x = 0xff;
        uint16 y = 0xffff;
        y |= x;
        assert(y == 0xff); // fails
        assert(y == 0xffff);

        y = 0xf1ff;
        x = 0xff00;
        x |= y & x;
        assert(y == 0xffff); // fails
        assert(x == 0xff00);
    }
}
// ----
// Warning 6328: (121-135): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (298-315): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (424-443): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
