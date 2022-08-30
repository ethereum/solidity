contract C {
    error E(uint[], uint[1]);

    // This case used to be affected by the buggy cleanup due to ABIEncoderV2HeadOverflowWithStaticArrayCleanup bug.
    function f(uint[] memory a, uint[1] calldata b) public {
        revert E(a, b);
    }
}
// ----
// f(uint256[],uint256[1]): 0x40, 0xff, 1, 0xffff -> FAILURE, hex"f42f106d", 0x40, 0xff, 1, 0xffff
