contract C {
    function f(uint[2][] calldata x) public returns (uint[2][] memory r) {
        assembly { x.offset := 0x44 x.length := 2 }
        r = x;
    }
}
// ====
// compileViaYul: also
// ----
// f(uint256[2][]): 0x0, 1, 8, 7, 6, 5 -> 0x20, 2, 8, 7, 6, 5
