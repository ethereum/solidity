contract Test {
    uint24[] public data;

    function set(uint24[] memory _data) public returns(uint) {
        data = _data;
        return data.length;
    }

    function get() public returns(uint24[] memory) {
        return data;
    }
}

// ----
set(uint24[]): "32, 18, ?" // data(uint256): 7) -> 8
// data(uint256):"7" -> "8"
// data(uint256): 15) -> 16
// data(uint256):"15" -> "16"
// data(uint256): 18) -> 
// data(uint256):"18" -> ""
// get() -> 0x20, data.size(, data
// get():"" -> "32, 18, ?"
