pragma experimental SMTChecker;
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
// ----
// Warning 6328: (259-273): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\nTransaction trace:\nC.constructor()\nState: x = 0\nA.proxy()\n    C.f() -- internal call
