contract C {
    function f(uint8 x, uint8 y) public returns (uint) {
        return x**y;
    }
    function g(uint x, uint y) public returns (uint) {
        return x**y;
    }
}
// ====
// compileViaYul: also
// ----
// f(uint8,uint8): 2, 7 -> 0x80
// f(uint8,uint8): 2, 8 -> FAILURE, hex"4e487b71", 0x11
// f(uint8,uint8): 15, 2 -> 225
// f(uint8,uint8): 6, 3 -> 0xd8
// f(uint8,uint8): 7, 2 -> 0x31
// f(uint8,uint8): 7, 3 -> FAILURE, hex"4e487b71", 0x11
// f(uint8,uint8): 7, 4 -> FAILURE, hex"4e487b71", 0x11
// f(uint8,uint8): 255, 31 -> FAILURE, hex"4e487b71", 0x11
// f(uint8,uint8): 255, 131 -> FAILURE, hex"4e487b71", 0x11
// g(uint256,uint256): 0x200000000000000000000000000000000, 1 -> 0x0200000000000000000000000000000000
// g(uint256,uint256): 0x100000000000000000000000000000010, 2 -> FAILURE, hex"4e487b71", 0x11
// g(uint256,uint256): 0x200000000000000000000000000000000, 2 -> FAILURE, hex"4e487b71", 0x11
// g(uint256,uint256): 0x200000000000000000000000000000000, 3 -> FAILURE, hex"4e487b71", 0x11
// g(uint256,uint256): 255, 31 -> 400631961586894742455537928461950192806830589109049416147172451019287109375
// g(uint256,uint256): 255, 32 -> -13630939032658036097408813250890608687528184442832962921928608997994916749311
// g(uint256,uint256): 255, 33 -> FAILURE, hex"4e487b71", 0x11
// g(uint256,uint256): 255, 131 -> FAILURE, hex"4e487b71", 0x11
// g(uint256,uint256): 258, 31 -> 575719427506838823084316385994930914701079543089399988096291424922125729792
// g(uint256,uint256): 258, 37 -> FAILURE, hex"4e487b71", 0x11
// g(uint256,uint256): 258, 131 -> FAILURE, hex"4e487b71", 0x11
