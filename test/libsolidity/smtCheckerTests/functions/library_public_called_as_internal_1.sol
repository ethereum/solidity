library L {
    function f1(uint x) public pure {
        assert(x > 0); // should fail
    }
    function f(uint x) internal pure { f1(x); }
}

contract C {
    function g(uint x) external pure {
        // This should trigger the assertion failure
        // since it calls `f` internally, which calls
        // `f1` internally.
        return L.f(x);
    }

    function h(uint x) external pure {
        // This should not trigger the assertion failure
        // since it delegatecalls and that's not supported.
        return L.f1(x);
    }
}
// ====
// SMTContract: C
// ----
// Warning 4588: (533-540): Assertion checker does not yet implement this type of function call.
// Warning 6328: (58-71): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\n\nTransaction trace:\nC.constructor()\nC.g(0)\n    L.f(0) -- internal call\n        L.f1(0) -- internal call
