contract Test {
    function set(uint24[3][] memory _data, uint a, uint b) public returns(uint l, uint e) {
        l = _data.length;
        e = _data[a][b];
    }
}

// ====
// compileViaYul: also
// ----
// set(uint24[3][],uint256,uint256): 0x60, 3, 2, 6, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 -> 6, 12
