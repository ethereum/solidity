contract c {
    uint[40] data1;
    uint[20] data2;

    function test() public returns(uint x, uint y) {
        data1[30] = 4;
        data1[2] = 7;
        data1[3] = 9;
        data2[3] = 8;
        data1 = data2;
        x = data1[3];
        y = data1[30]; // should be cleared
    }
}

// ----
// test() -> 8, 0
// test():"" -> "8, 0"
