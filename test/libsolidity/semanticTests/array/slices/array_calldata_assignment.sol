contract C {
    function f(uint256[] calldata x, uint256[] calldata y, uint256 i) external returns (uint256) {
        x = y;
        return x[i];
    }
}
// ====
// compileToEwasm: also
// ----
// f(uint256[],uint256[],uint256): 0x60, 0xA0, 1, 1, 0, 2, 1, 2 -> 2
