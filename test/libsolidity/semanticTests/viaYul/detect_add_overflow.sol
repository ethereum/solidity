contract C {
    function f(uint a, uint b) public pure returns (uint x) {
        x = a + b;
    }
    function g(uint8 a, uint8 b) public pure returns (uint8 x) {
        x = a + b;
    }
}
// ====
// compileViaYul: true
// ----
// f(uint256,uint256): 5, 6 -> 11
// f(uint256,uint256): -2, 1 -> -1
// f(uint256,uint256): -2, 2 -> FAILURE
// f(uint256,uint256): 2, -2 -> FAILURE
// g(uint8,uint8): 128, 64 -> 192
// g(uint8,uint8): 128, 127 -> 255
// g(uint8,uint8): 128, 128 -> FAILURE
