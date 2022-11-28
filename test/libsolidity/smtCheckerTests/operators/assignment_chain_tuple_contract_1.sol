contract C {
    string public name = (C).name = type(C).name;

    function f() external view {
        assert(keccak256(bytes(name)) == keccak256(bytes("C"))); // should fail because SMTChecker doesn't support type(C).name
    }
}
// ----
// Warning 7507: (49-61): Assertion checker does not yet support this expression.
// Warning 6328: (105-160): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
