contract C {
    function f(uint a, uint b) public pure returns (uint x) {
        x = a / b;
    }
    function g(int8 a, int8 b) public pure returns (int8 x) {
        x = a / b;
    }
    function h(uint256 a, uint256 b) public pure returns (uint256 x) {
        x = a / b;
    }
}
// ====
// compileViaYul: true
// ----
// f(uint256,uint256): 10, 3 -> 3
// f(uint256,uint256): 1, 0 -> FAILURE
// f(uint256,uint256): 0, 0 -> FAILURE
// f(uint256,uint256): 0, 1 -> 0
// g(int8,int8): -10, 3 -> -3
// g(int8,int8): -10, -3 -> 3
// g(int8,int8): -10, 0 -> FAILURE
// g(int8,int8): -128, 1 -> -128
// g(int8,int8): -128, -2 -> 64
// g(int8,int8): -128, 2 -> -64
// g(int8,int8): -128, -1 -> FAILURE
// g(int8,int8): -127, -1 -> 127
// h(uint256,uint256): 0x8000000000000000000000000000000000000000000000000000000000000000, -1 -> 0
