pragma experimental SMTChecker;

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
// ----
// Warning 6328: (177-191): CHC: Assertion violation happens here.\nCounterexample:\n\nv = 3\nc = 0\nx = 0\ny = 0\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (380-399): CHC: Assertion violation happens here.\nCounterexample:\n\nv = 3\nc = 0\nx = 255\ny = 255\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (506-523): CHC: Assertion violation happens here.\nCounterexample:\n\nv = 3\nc = 0\nx = 255\ny = 65535\n\nTransaction trace:\nC.constructor()\nC.f()
