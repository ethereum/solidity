pragma experimental SMTChecker;

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
// ----
// Warning 6031: (261-267): Internal error: Expression undefined for SMT solver.
// Warning 7650: (284-296): Assertion checker does not yet support this expression.
// Warning 6328: (470-495): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0\ncheck()
// Warning 6328: (540-565): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0\ncheck()
// Warning 6031: (261-267): Internal error: Expression undefined for SMT solver.
// Warning 7650: (284-296): Assertion checker does not yet support this expression.
// Warning 7650: (284-296): Assertion checker does not yet support this expression.
