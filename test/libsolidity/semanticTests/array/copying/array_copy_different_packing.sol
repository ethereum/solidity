contract c {
    bytes8[] data1; // 4 per slot
    bytes10[] data2; // 3 per slot

    function test()
        public
        returns (bytes10 a, bytes10 b, bytes10 c, bytes10 d, bytes10 e)
    {
        data1 = new bytes8[](9);
        for (uint256 i = 0; i < data1.length; ++i) data1[i] = bytes8(uint64(i));
        data2 = data1;
        a = data2[1];
        b = data2[2];
        c = data2[3];
        d = data2[4];
        e = data2[5];
    }
}

// ----
// test() -> 0x01000000000000000000000000000000000000000000000000, 0x02000000000000000000000000000000000000000000000000, 0x03000000000000000000000000000000000000000000000000, 0x04000000000000000000000000000000000000000000000000, 0x05000000000000000000000000000000000000000000000000
// gas irOptimized: 207960
// gas legacy: 220776
// gas legacyOptimized: 220158
