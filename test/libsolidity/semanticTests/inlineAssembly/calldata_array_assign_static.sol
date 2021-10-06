contract C {
    function f(uint[2][2] calldata x) public returns (uint[2][2] memory r) {
        assembly { x := 0x24 }
        r = x;
    }
}
// ====
// compileViaYul: also
// ----
// f(uint256[2][2]): 0x0, 8, 7, 6, 5 -> 8, 7, 6, 5
