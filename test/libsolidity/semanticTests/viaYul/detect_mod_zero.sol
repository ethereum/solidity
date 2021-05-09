contract C {
    function f(uint a, uint b) public pure returns (uint x) {
        x = a % b;
    }
    function g(uint8 a, uint8 b) public pure returns (uint8 x) {
        x = a % b;
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f(uint256,uint256): 10, 3 -> 1
// f(uint256,uint256): 10, 2 -> 0
// f(uint256,uint256): 11, 2 -> 1
// f(uint256,uint256): 2, 2 -> 0
// f(uint256,uint256): 1, 0 -> FAILURE, hex"4e487b71", 0x12
// f(uint256,uint256): 0, 0 -> FAILURE, hex"4e487b71", 0x12
// f(uint256,uint256): 0, 1 -> 0
// g(uint8,uint8): 10, 3 -> 1
// g(uint8,uint8): 10, 2 -> 0
// g(uint8,uint8): 11, 2 -> 1
// g(uint8,uint8): 2, 2 -> 0
// g(uint8,uint8): 1, 0 -> FAILURE, hex"4e487b71", 0x12
// g(uint8,uint8): 0, 0 -> FAILURE, hex"4e487b71", 0x12
// g(uint8,uint8): 0, 1 -> 0
