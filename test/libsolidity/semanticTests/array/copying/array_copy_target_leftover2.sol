// since the copy always copies whole slots, we have to make sure that the source size maxes
// out a whole slot and at the same time there are still elements left in the target at that point
contract c {
    bytes8[4] data1; // fits into one slot
    bytes10[6] data2; // 4 elements need two slots

    function test() public returns (bytes10 r1, bytes10 r2, bytes10 r3) {
        data1[0] = bytes8(uint64(1));
        data1[1] = bytes8(uint64(2));
        data1[2] = bytes8(uint64(3));
        data1[3] = bytes8(uint64(4));
        for (uint256 i = 0; i < data2.length; ++i)
            data2[i] = bytes10(uint80(0xffff00 | (1 + i)));
        data2 = data1;
        r1 = data2[3];
        r2 = data2[4];
        r3 = data2[5];
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// test() -> 0x04000000000000000000000000000000000000000000000000, 0x0, 0x0
// gas irOptimized: 93867
// gas legacy: 97451
// gas legacyOptimized: 94110
