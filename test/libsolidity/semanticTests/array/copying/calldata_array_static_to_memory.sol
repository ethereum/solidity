contract C {
    function f(uint256[2] calldata c) public returns (uint256, uint256) {
        uint256[2] memory m1 = c;
        return (m1[0], m1[1]);
    }
}
// ====
// requiresYulOptimizer: minimalStack
// compileToEwasm: also
// compileViaYul: also
// ----
// f(uint256[2]): 43, 57 -> 43, 57
