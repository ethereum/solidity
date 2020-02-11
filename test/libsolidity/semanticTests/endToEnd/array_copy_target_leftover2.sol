contract c {
    bytes8[4] data1; // fits into one slot
    bytes10[6] data2; // 4 elements need two slots
    function test() public returns(bytes10 r1, bytes10 r2, bytes10 r3) {
        data1[0] = bytes8(uint64(1));
        data1[1] = bytes8(uint64(2));
        data1[2] = bytes8(uint64(3));
        data1[3] = bytes8(uint64(4));
        for (uint i = 0; i < data2.length; ++i)
            data2[i] = bytes10(uint80(0xffff00 | (1 + i)));
        data2 = data1;
        r1 = data2[3];
        r2 = data2[4];
        r3 = data2[5];
    }
}

// ----
// test() ->  hex"0000000000000004", hex"0000000000000000", hex"0000000000000000" 
// test():"" -> "\0\0\0\0\0\0\0\x0, \0\0\0\0\0\0\0\0, \0\0\0\0\0\0\0\0"
