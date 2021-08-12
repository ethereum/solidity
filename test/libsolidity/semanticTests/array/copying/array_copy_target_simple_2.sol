contract c {
    bytes9[7] data1; // 3 per slot
    bytes32[10] data2; // 1 per slot

    function test()
        public
        returns (bytes32 a, bytes32 b, bytes32 c, bytes32 d, bytes32 e)
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
// compileViaYul: also
// ----
// test() -> 0x01000000000000000000000000000000000000000000000000, 0x02000000000000000000000000000000000000000000000000, 0x03000000000000000000000000000000000000000000000000, 0x04000000000000000000000000000000000000000000000000, 0x00
// gas irOptimized: 273963
// gas legacy: 276381
// gas legacyOptimized: 275390
