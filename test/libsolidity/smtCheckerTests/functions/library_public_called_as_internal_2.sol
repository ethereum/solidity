library L {
    function f1(uint x) public pure {
        assert(x > 0); // should fail
    }
    function f() internal pure {
        f1(0); // should cause the assertion in `f1` to fail
    }
    function g() internal pure {
        f1(1); // should not cause the assertion in `f1` to fail
    }
}

contract C {
    function f() external pure {
        return L.f(); // should cause the assertion to fail
    }
    function g() external pure {
        return L.g(); // should not cause the assertion to fail
    }
}
// ====
// SMTContract: C
// ----
// Warning 6328: (58-71): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()\n    L.f() -- internal call\n        L.f1(0) -- internal call
