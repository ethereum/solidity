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
set(uint24[3][]): "32, 6, ?" // data(uint256,uint256): 2), 2) -> 9
// data(uint256,uint256):"2, 2" -> "9"
// data(uint256,uint256): 5), 1) -> 17
// data(uint256,uint256):"5, 1" -> "17"
// data(uint256,uint256): 6), 0) -> 
// data(uint256,uint256):"6, 0" -> ""
// get() -> 0x20, data.size( / 3, data
// get():"" -> "32, 6, ?"
