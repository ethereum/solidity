uint256 constant ONE = 1;
uint256 constant A = ONE << 300;   // exact 2^300 fits the 4096-bit precision cap
uint256 constant B = ONE << 5000;  // exact 2^5000 exceeds it

contract C {
    uint256[A + 1] a;
    uint256[B + 1] b;
    function testRuntimeEquivalence() public view returns (bool) {
        uint256 runtimeResultA = ONE << 300;
        uint256 runtimeResultB = ONE << 5000;
        return
            A == runtimeResultA &&
            B == runtimeResultB &&
            a.length == 1 &&
            b.length == 1;
    }
}
// ----
// testRuntimeEquivalence() -> true
