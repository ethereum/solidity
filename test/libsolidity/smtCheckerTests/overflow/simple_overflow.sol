pragma experimental SMTChecker;
contract C {
    function f(uint a, uint b) public pure returns (uint) { return a + b; }
}
// ----
// Warning: (112-117): Overflow (resulting value larger than 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) happens here
