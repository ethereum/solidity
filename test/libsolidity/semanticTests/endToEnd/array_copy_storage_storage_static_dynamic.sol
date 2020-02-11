contract c {
    uint[9] data1;
    uint[] data2;

    function test() public returns(uint x, uint y) {
        data1[8] = 4;
        data2 = data1;
        x = data2.length;
        y = data2[8];
    }
}

// ----
// test() -> 9, 4
// test():"" -> "9, 4"
