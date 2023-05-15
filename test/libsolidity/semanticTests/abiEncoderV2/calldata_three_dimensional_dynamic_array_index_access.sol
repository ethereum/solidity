pragma abicoder v2;

contract C {
    struct S { uint[] a; }

    function f(uint[][] calldata s, uint i, uint j) public pure returns (bytes memory) {
        return abi.encode(s[i][j]);
    }

    function g(uint[][][] calldata s, uint i, uint j, uint k) public pure returns (bytes memory) {
        return abi.encode(s[i][j][k]);
    }

    function h(uint[][][1] calldata s, uint i) public pure returns (bytes memory) {
        return abi.encode(s[0][i]);
    }

    function k(S[][] calldata s, uint i, uint j) public pure returns (bytes memory) {
        return abi.encode(s[i][j].a);
    }

    function l(S[2][2] calldata s, uint i, uint j) public pure returns (bytes memory) {
        return abi.encode(s[i][j].a);
    }
}
// ====
// revertStrings: debug
// ----
// f(uint256[][],uint256,uint256): 0x60, 0, 0, 2, 0x40, 0x80, 1, 7, 1, 8 -> 0x20, 0x20, 7
// f(uint256[][],uint256,uint256): 0x60, 1, 0, 2, 0x40, 0x80, 1, 7, 1, 8 -> 0x20, 0x20, 8
// g(uint256[][][],uint256,uint256,uint256): 0x80, 0, 0, 0, 2, 0x40, 0xc0, 1, 0x20, 1, 4, 2, 0x40, 0xa0, 2, 5, 6, 1, 7 -> 0x20, 0x20, 4
// g(uint256[][][],uint256,uint256,uint256): 0x80, 1, 0, 1, 2, 0x40, 0xc0, 1, 0x20, 1, 4, 2, 0x40, 0xa0, 2, 5, 6, 1, 7 -> 0x20, 0x20, 6
// g(uint256[][][],uint256,uint256,uint256): 0x80, 1, 0, 2, 2, 0x40, 0xc0, 1, 0x20, 1, 4, 2, 0x40, 0xa0, 2, 5, 6, 1, 7 -> FAILURE, hex"4e487b71", 0x32
// g(uint256[][][],uint256,uint256,uint256): 0x80, 2, 0, 1, 2, 0x40, 0xc0, 1, 0x20, 1, 4, 2, 0x40, 0xa0, 2, 5, 6, 1, 7 -> FAILURE, hex"4e487b71", 0x32
// h(uint256[][][1],uint256): 0x40, 1, 0x20, 2, 0x40, 0xA0, 2, 5, 6, 3, 7, 8, 9 -> 0x20, 0xa0, 0x20, 3, 7, 8, 9
// h(uint256[][][1],uint256): 0x40, 2, 0x20, 2, 0x40, 0xA0, 2, 5, 6, 3, 7, 8, 9 -> FAILURE, hex"4e487b71", 0x32
// k((uint256[])[][],uint256,uint256): 0x60, 0, 0, 2, 0x40, 0xe0, 1, 0x20, 0x20, 1, 6, 2, 0x40, 0xa0, 0x20, 1, 7, 0x20, 2, 8, 9 -> 0x20, 0x60, 0x20, 1, 6
// k((uint256[])[][],uint256,uint256): 0x60, 0, 1, 2, 0x40, 0xe0, 1, 0x20, 0x20, 1, 6, 2, 0x40, 0xa0, 0x20, 1, 7, 0x20, 2, 8, 9 -> FAILURE, hex"4e487b71", 0x32
// l((uint256[])[2][2],uint256,uint256): 0x60, 1, 1, 0x40, 0x0140, 0x40, 0xa0, 0x20, 1, 5, 0x20, 1, 6, 0x40, 0xa0, 0x20, 1, 7, 0x20, 2, 8, 9 -> 0x20, 0x80, 0x20, 2, 8, 9
// l((uint256[])[2][2],uint256,uint256): 0x60, 1, 2, 0x40, 0x0140, 0x40, 0xa0, 0x20, 1, 5, 0x20, 1, 6, 0x40, 0xa0, 0x20, 1, 7, 0x20, 2, 8, 9 -> FAILURE, hex"4e487b71", 0x32
