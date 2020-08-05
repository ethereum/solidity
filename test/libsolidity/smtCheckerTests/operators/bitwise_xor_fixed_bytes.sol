pragma experimental SMTChecker;
contract Simp {
    function f3() public pure returns (byte) {
        bytes memory y = "def";
        return y[0] ^ "e";
    }
}
// ----
// Warning 1093: (142-152): Assertion checker does not yet implement this bitwise operator.
