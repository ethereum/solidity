pragma experimental SMTChecker;

contract C {
    function f() public pure returns (bool) {
        bool b;
        bool c = true;
        assembly { b := c }
        assert(c); // should hold, c is not assigned in the assembly
        assert(b); // should hold, but fails currently because of overapproximation
        return b;
    }
}
// ----
// Warning 7737: (139-158): Inline assembly may cause SMTChecker to produce spurious warnings (false positives).
// Warning 6328: (236-245): CHC: Assertion violation happens here.\nCounterexample:\n\n = false\nb = false\nc = true\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 7737: (139-158): Inline assembly may cause SMTChecker to produce spurious warnings (false positives).
