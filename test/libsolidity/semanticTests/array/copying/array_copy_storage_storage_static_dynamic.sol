contract c {
    uint256[9] data1;
    uint256[] data2;

    function test() public returns (uint256 x, uint256 y) {
        data1[8] = 4;
        data2 = data1;
        x = data2.length;
        y = data2[8];
    }
}

// ----
// test() -> 9, 4
