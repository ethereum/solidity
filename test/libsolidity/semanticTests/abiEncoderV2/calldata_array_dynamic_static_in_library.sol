library L {
    // This case used to be affected by the buggy cleanup due to ABIEncoderV2HeadOverflowWithStaticArrayCleanup bug.
    function g(uint[] memory a, uint[1] calldata b) public returns (uint[] memory, uint[1] calldata) {
        return (a, b);
    }
}

contract C {
    function f(uint[] memory a, uint[1] calldata b) public returns (uint[] memory, uint[1] memory) {
        return L.g(a, b);
    }
}
// ====
// EVMVersion: >homestead
// ----
// library: L
// f(uint256[],uint256[1]): 0x40, 0xff, 1, 0xffff -> 0x40, 0xff, 0x01, 0xffff
