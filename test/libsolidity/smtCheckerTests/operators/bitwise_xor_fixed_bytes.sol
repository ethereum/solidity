pragma experimental SMTChecker;
contract Simp {
    function f3() public pure returns (byte) {
        bytes memory y = "def";
        return y[0] ^ "e";
    }
}
// ----
