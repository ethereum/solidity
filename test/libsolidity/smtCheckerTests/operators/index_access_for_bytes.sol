pragma experimental SMTChecker;
contract C {
    bytes20 x;
    function f(bytes16 b) public view {
        b[uint8(x[2])];
    }
}
// ----
// Warning: (116-120): Assertion checker does not yet support index accessing fixed bytes.
// Warning: (108-122): Assertion checker does not yet support index accessing fixed bytes.
