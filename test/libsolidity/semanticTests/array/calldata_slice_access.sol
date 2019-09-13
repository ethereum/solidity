contract C {
    function f(uint256[] calldata x, uint256 start, uint256 end) external pure {
        x[start:end];
    }
    function g(uint256[] calldata x, uint256 start, uint256 end, uint256 index) external pure returns (uint256, uint256, uint256) {
        return (x[start:end][index], x[start:][0:end-start][index], x[:end][start:][index]);
    }
}
// ----
// f(uint256[],uint256,uint256): 0x80, 0, 0, 0, 1, 42 ->
// f(uint256[],uint256,uint256): 0x80, 0, 1, 0, 1, 42 ->
// f(uint256[],uint256,uint256): 0x80, 0, 2, 0, 1, 42 -> FAILURE
// f(uint256[],uint256,uint256): 0x80, 1, 0, 0, 1, 42 -> FAILURE
// f(uint256[],uint256,uint256): 0x80, 1, 1, 0, 1, 42 ->
// f(uint256[],uint256,uint256): 0x80, 1, 2, 0, 1, 42 -> FAILURE
// f(uint256[],uint256,uint256): 0x80, 2, 0, 0, 1, 42 -> FAILURE
// f(uint256[],uint256,uint256): 0x80, 2, 1, 0, 1, 42 -> FAILURE
// f(uint256[],uint256,uint256): 0x80, 2, 2, 0, 1, 42 -> FAILURE
// f(uint256[],uint256,uint256): 0x80, 0, 2, 1, 0, 42 -> FAILURE
// f(uint256[],uint256,uint256): 0x80, 1, 2, 0, 2, 42, 23 ->
// f(uint256[],uint256,uint256): 0x80, 1, 3, 0, 2, 42, 23 -> FAILURE
// f(uint256[],uint256,uint256): 0x80, -1, 0, 0, 1, 42 -> FAILURE
// f(uint256[],uint256,uint256): 0x80, -1, -1, 0, 1, 42 -> FAILURE
// g(uint256[],uint256,uint256,uint256): 0x80, 0, 1, 0, 1, 42 -> 42, 42, 42
// g(uint256[],uint256,uint256,uint256): 0x80, 0, 1, 1, 1, 42 -> FAILURE
// g(uint256[],uint256,uint256,uint256): 0x80, 0, 0, 0, 1, 42 -> FAILURE
// g(uint256[],uint256,uint256,uint256): 0x80, 1, 1, 0, 1, 42 -> FAILURE
// g(uint256[],uint256,uint256,uint256): 0x80, 0, 5, 0, 5, 0x4201, 0x4202, 0x4203, 0x4204, 0x4205 -> 0x4201, 0x4201, 0x4201
// g(uint256[],uint256,uint256,uint256): 0x80, 0, 5, 4, 5, 0x4201, 0x4202, 0x4203, 0x4204, 0x4205 -> 0x4205, 0x4205, 0x4205
// g(uint256[],uint256,uint256,uint256): 0x80, 0, 5, 5, 5, 0x4201, 0x4202, 0x4203, 0x4204, 0x4205 -> FAILURE
// g(uint256[],uint256,uint256,uint256): 0x80, 1, 5, 0, 5, 0x4201, 0x4202, 0x4203, 0x4204, 0x4205 -> 0x4202, 0x4202, 0x4202
// g(uint256[],uint256,uint256,uint256): 0x80, 1, 5, 3, 5, 0x4201, 0x4202, 0x4203, 0x4204, 0x4205 -> 0x4205, 0x4205, 0x4205
// g(uint256[],uint256,uint256,uint256): 0x80, 1, 5, 4, 5, 0x4201, 0x4202, 0x4203, 0x4204, 0x4205 -> FAILURE
// g(uint256[],uint256,uint256,uint256): 0x80, 4, 5, 0, 5, 0x4201, 0x4202, 0x4203, 0x4204, 0x4205 -> 0x4205, 0x4205, 0x4205
// g(uint256[],uint256,uint256,uint256): 0x80, 4, 5, 1, 5, 0x4201, 0x4202, 0x4203, 0x4204, 0x4205 -> FAILURE
// g(uint256[],uint256,uint256,uint256): 0x80, 5, 5, 0, 5, 0x4201, 0x4202, 0x4203, 0x4204, 0x4205 -> FAILURE
// g(uint256[],uint256,uint256,uint256): 0x80, 0, 1, 0, 5, 0x4201, 0x4202, 0x4203, 0x4204, 0x4205 -> 0x4201, 0x4201, 0x4201
// g(uint256[],uint256,uint256,uint256): 0x80, 0, 1, 1, 5, 0x4201, 0x4202, 0x4203, 0x4204, 0x4205 -> FAILURE
// g(uint256[],uint256,uint256,uint256): 0x80, 0, 1, 0, 5, 0x4201, 0x4202, 0x4203, 0x4204, 0x4205 -> 0x4201, 0x4201, 0x4201
// g(uint256[],uint256,uint256,uint256): 0x80, 0, 1, 1, 5, 0x4201, 0x4202, 0x4203, 0x4204, 0x4205 -> FAILURE
// g(uint256[],uint256,uint256,uint256): 0x80, 1, 2, 0, 5, 0x4201, 0x4202, 0x4203, 0x4204, 0x4205 -> 0x4202, 0x4202, 0x4202
// g(uint256[],uint256,uint256,uint256): 0x80, 1, 2, 1, 5, 0x4201, 0x4202, 0x4203, 0x4204, 0x4205 -> FAILURE
// g(uint256[],uint256,uint256,uint256): 0x80, 4, 5, 0, 5, 0x4201, 0x4202, 0x4203, 0x4204, 0x4205 -> 0x4205, 0x4205, 0x4205
// g(uint256[],uint256,uint256,uint256): 0x80, 4, 5, 1, 5, 0x4201, 0x4202, 0x4203, 0x4204, 0x4205 -> FAILURE
