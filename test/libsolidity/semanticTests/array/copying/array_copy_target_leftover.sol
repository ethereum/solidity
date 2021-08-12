contract c {
    bytes1[10] data1;
    bytes2[32] data2;
    function test() public returns (uint check, uint res1, uint res2) {
        uint i;
        for (i = 0; i < data2.length; ++i)
            data2[i] = 0xffff;
        check = uint(uint16(data2[31])) * 0x10000 | uint(uint16(data2[14]));
        for (i = 0; i < data1.length; ++i)
            data1[i] = bytes1(uint8(1 + i));
        data2 = data1;
        for (i = 0; i < 16; ++i)
            res1 |= uint(uint16(data2[i])) * 0x10000**i;
        for (i = 0; i < 16; ++i)
            res2 |= uint(uint16(data2[16 + i])) * 0x10000**i;
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// test() -> 0xffffffff, 0x0000000000000000000000000a00090008000700060005000400030002000100, 0x0000000000000000000000000000000000000000000000000000000000000000
// gas irOptimized: 129197
// gas legacy: 186406
// gas legacyOptimized: 163309
