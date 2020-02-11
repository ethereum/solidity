contract c {
    byte[10] data1;
    bytes2[32] data2;

    function test() public returns(uint check, uint res1, uint res2) {
        uint i;
        for (i = 0; i < data2.length; ++i)
            data2[i] = 0xffff;
        check = uint(uint16(data2[31])) * 0x10000 | uint(uint16(data2[14]));
        for (i = 0; i < data1.length; ++i)
            data1[i] = byte(uint8(1 + i));
        data2 = data1;
        for (i = 0; i < 16; ++i)
            res1 |= uint(uint16(data2[i])) * 0x10000 ** i;
        for (i = 0; i < 16; ++i)
            res2 |= uint(uint16(data2[16 + i])) * 0x10000 ** i;
    }
}

// ----
// test() ->  "0xffffffff", hex"0000000000000000""000000000a000900""0800070006000500""0400030002000100", hex"0000000000000000""0000000000000000""0000000000000000""0000000000000000" 
// test():"" -> "4294967295, \0\0\0\0\0\0\0\0\0\0\0\0\n\0\x0	\0\x0\0\x0\0\x0\0\x0\0\x0\0\x0\0\x0\0\x0\0, \0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
