contract A {
    int x = 0;

    function f() virtual internal view {
        assert(x == 0);
    }

    function proxy() public view {
        f();
    }
}

contract C is A {

    function f() internal view override {
        assert(x == 1);
    }
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (227-241): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\nTransaction trace:\nC.constructor()\nState: x = 0\nA.proxy()\n    C.f() -- internal call
// Info 1180: Contract invariant(s) for :A:\n((x >= 0) && (x <= 0))\n
