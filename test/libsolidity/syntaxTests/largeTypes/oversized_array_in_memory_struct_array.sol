contract C {
    struct S { uint256[134217729] y; }
    function f() public pure returns (bytes memory) {
        S[1] memory s;
        return abi.encode(s);
    }
}
// ----
// TypeError 1534: (114-127): Type too large for memory.
