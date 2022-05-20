contract C {
    function f(uint8 x, uint8 y) public returns (uint) {
        return x**y;
    }
    function g(uint x, uint y) public returns (uint) {
        return x**y;
    }
}
// ====
// compileToEwasm: also
// ----
// f(uint8,uint8): 0, 0 -> 1
// f(uint8,uint8): 0, 1 -> 0x00
// f(uint8,uint8): 0, 2 -> 0x00
// f(uint8,uint8): 0, 3 -> 0x00
// f(uint8,uint8): 1, 0 -> 1
// f(uint8,uint8): 1, 1 -> 1
// f(uint8,uint8): 1, 2 -> 1
// f(uint8,uint8): 1, 3 -> 1
// f(uint8,uint8): 2, 0 -> 1
// f(uint8,uint8): 2, 1 -> 2
// f(uint8,uint8): 2, 2 -> 4
// f(uint8,uint8): 2, 3 -> 8
// f(uint8,uint8): 3, 0 -> 1
// f(uint8,uint8): 3, 1 -> 3
// f(uint8,uint8): 3, 2 -> 9
// f(uint8,uint8): 3, 3 -> 0x1b
// f(uint8,uint8): 10, 0 -> 1
// f(uint8,uint8): 10, 1 -> 0x0a
// f(uint8,uint8): 10, 2 -> 100
// g(uint256,uint256): 0, 0 -> 1
// g(uint256,uint256): 0, 1 -> 0x00
// g(uint256,uint256): 0, 2 -> 0x00
// g(uint256,uint256): 0, 3 -> 0x00
// g(uint256,uint256): 1, 0 -> 1
// g(uint256,uint256): 1, 1 -> 1
// g(uint256,uint256): 1, 2 -> 1
// g(uint256,uint256): 1, 3 -> 1
// g(uint256,uint256): 2, 0 -> 1
// g(uint256,uint256): 2, 1 -> 2
// g(uint256,uint256): 2, 2 -> 4
// g(uint256,uint256): 2, 3 -> 8
// g(uint256,uint256): 3, 0 -> 1
// g(uint256,uint256): 3, 1 -> 3
// g(uint256,uint256): 3, 2 -> 9
// g(uint256,uint256): 3, 3 -> 0x1b
// g(uint256,uint256): 10, 10 -> 10000000000
// g(uint256,uint256): 10, 77 -> -15792089237316195423570985008687907853269984665640564039457584007913129639936
// g(uint256,uint256): 256, 2 -> 0x010000
// g(uint256,uint256): 256, 31 -> 0x0100000000000000000000000000000000000000000000000000000000000000
