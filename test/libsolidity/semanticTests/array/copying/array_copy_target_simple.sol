contract c {
    bytes8[9] data1; // 4 per slot
    bytes17[10] data2; // 1 per slot, no offset counter

    function test()
        public
        returns (bytes17 a, bytes17 b, bytes17 c, bytes17 d, bytes17 e)
    {
        for (uint256 i = 0; i < data1.length; ++i) data1[i] = bytes8(uint64(i));
        data2[8] = data2[9] = bytes8(uint64(2));
        data2 = data1;
        a = data2[1];
        b = data2[2];
        c = data2[3];
        d = data2[4];
        e = data2[9];
    }
}

// ====
// compileToEwasm: also
// ----
// test() -> 0x01000000000000000000000000000000000000000000000000, 0x02000000000000000000000000000000000000000000000000, 0x03000000000000000000000000000000000000000000000000, 0x04000000000000000000000000000000000000000000000000, 0x0
// gas irOptimized: 293420
// gas legacy: 302546
// gas legacyOptimized: 301423
