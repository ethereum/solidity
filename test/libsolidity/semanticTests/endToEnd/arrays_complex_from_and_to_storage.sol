contract Test {
    uint24[3][] public data;

    function set(uint24[3][] memory _data) public returns(uint) {
        data = _data;
        return data.length;
    }

    function get() public returns(uint24[3][] memory) {
        return data;
    }
}

// ----
// set(uint24[3][]): 0x20, 6, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 -> 6
// data(uint256,uint256): 2, 2 -> 9
// data(uint256,uint256): 5, 1 -> 17
// data(uint256,uint256): 6, 0 -> FAILURE
// get() -> 0x20, 6, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18
