pragma experimental SMTChecker;
contract C {
    function f() public pure returns (uint) {
        return uint8(1);
    }
}
// ----
// Warning: (106-114): Assertion checker does not yet implement this expression.
