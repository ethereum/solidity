contract C {
    function f(uint a, uint b) public pure returns (uint x) {
        x = a - b;
    }
    function g(uint8 a, uint8 b) public pure returns (uint8 x) {
        x = a - b;
    }
}
// ====
// compileViaYul: true
// ----
// f(uint256,uint256): 6, 5 -> 1
// f(uint256,uint256): 6, 6 -> 0
// f(uint256,uint256): 5, 6 -> FAILURE
// g(uint8,uint8): 6, 5 -> 1
// g(uint8,uint8): 6, 6 -> 0
// g(uint8,uint8): 5, 6 -> FAILURE
