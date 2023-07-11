pragma abicoder v2;

contract C {
    struct D { uint[] x; }
    struct S { uint x; }

    function f(D calldata a) public returns (bytes memory){
        return abi.encode(a);
    }

    function g(D[2] calldata a) public returns (bytes memory) {
        return abi.encode(a);
    }

    function h(D[][] calldata a) public returns (bytes memory) {
        return abi.encode(a);
    }

    function i(D[2][] calldata a) public returns (bytes memory) {
        return abi.encode(a);
    }

    function j(S[] memory a) public returns (bytes memory) {
        return abi.encode(a);
    }

    function k(S[2] memory a) public returns (bytes memory) {
        return abi.encode(a);
    }

    function l(S[][] memory a) public returns (bytes memory) {
        return abi.encode(a);
    }

}
// ----
// f((uint256[])): 0x20, 0x20, 0 -> 0x20, 0x60, 0x20, 0x20, 0
// f((uint256[])): 0x20, 0x20, 1 -> FAILURE
// f((uint256[])): 0x20, 0x20, 2 -> FAILURE
// f((uint256[])): 0x20, 0x20, 3 -> FAILURE
// g((uint256[])[2]): 0x20, 0x40, 0xc0, 0x20, 2, 1, 2, 0x20, 1, 3 -> 0x20, 0x0140, 0x20, 0x40, 0xc0, 0x20, 2, 1, 2, 0x20, 1, 3
// g((uint256[])[2]): 0x20, 0x40, 0xc0, 0x20, 2, 1, 2, 0x20, 1 -> FAILURE
// h((uint256[])[][]): 0x20, 0x02, 0x40, 0x0180, 2, 0x40, 0xc0, 0x20, 2, 1, 2, 0x20, 1, 3, 1, 0x20, 0x20, 1, 1 -> 0x20, 0x0260, 0x20, 2, 0x40, 0x0180, 2, 0x40, 0xc0, 0x20, 2, 1, 2, 0x20, 1, 3, 1, 0x20, 0x20, 1, 1
// h((uint256[])[][]): 0x20, 0x02, 0x40, 0x0180, 2, 0x40, 0xc0, 0x20, 2, 1, 2, 0x20, 1, 3, 1, 0x20, 0x20, 1 -> FAILURE
// i((uint256[])[2][]): 0x20, 1, 0x20, 0x40, 0xc0, 0x20, 2, 1, 2, 0x20, 1, 3 -> 0x20, 0x0180, 0x20, 1, 0x20, 0x40, 0xc0, 0x20, 2, 1, 2, 0x20, 1, 3
// i((uint256[])[2][]): 0x20, 1, 0x20, 0x40, 0xc0, 0x20, 2, 1, 2, 0x20, 1 -> FAILURE
// j((uint256)[]): 0x20, 2, 1, 2 -> 0x20, 0x80, 0x20, 2, 1, 2
// j((uint256)[]): 0x20, 2, 1 -> FAILURE
// k((uint256)[2]): 1, 2 -> 0x20, 0x40, 1, 2
// k((uint256)[2]): 1 -> FAILURE
// l((uint256)[][]): 0x20, 2, 0x40, 0xa0, 2, 5, 6, 3, 7, 8, 9 -> 0x20, 0x0160, 0x20, 2, 0x40, 0xa0, 2, 5, 6, 3, 7, 8, 9
// l((uint256)[][]): 0x20, 2, 0x40, 0xa0, 2, 5, 6, 3, 7, 8, 9, 10 -> 0x20, 0x0160, 0x20, 2, 0x40, 0xa0, 2, 5, 6, 3, 7, 8, 9
// l((uint256)[][]): 0x20, 2, 0x40, 0xa0, 2, 5, 6, 3, 7, 8 -> FAILURE
