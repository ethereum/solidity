contract C {
    uint256 public x;

    function f() public pure returns (bytes4) {
        return this.f.selector;
    }

    function g() public view returns (bytes4) {
        function () pure external returns (bytes4) fun = this.f;
        return fun.selector;
    }

    function i() public pure returns (bytes4) {
        return this.x.selector;
    }

    function check() public view {
        assert(f() == 0x26121ff0);
        assert(g() == 0x26121ff0);
        assert(i() == 0x0c55699c);
        assert(i() == 0x26121ff0);
    }
}
// ====
// SMTEngine: all
// ----
// Warning 7650: (251-263): Assertion checker does not yet support this expression.
// Warning 6328: (437-462): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\nTransaction trace:\nC.constructor()\nState: x = 0\nC.check()\n    C.f() -- internal call\n    C.g() -- internal call
// Warning 6328: (507-532): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\nTransaction trace:\nC.constructor()\nState: x = 0\nC.check()\n    C.f() -- internal call\n    C.g() -- internal call\n    C.i() -- internal call\n    C.i() -- internal call
// Warning 7650: (251-263): Assertion checker does not yet support this expression.
// Warning 7650: (251-263): Assertion checker does not yet support this expression.
