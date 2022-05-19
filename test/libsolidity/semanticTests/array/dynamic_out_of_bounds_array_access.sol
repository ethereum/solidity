contract c {
    uint256[] data;

    function enlarge(uint256 amount) public returns (uint256) {
        while (data.length < amount) data.push();
        return data.length;
    }

    function set(uint256 index, uint256 value) public returns (bool) {
        data[index] = value;
        return true;
    }

    function get(uint256 index) public returns (uint256) {
        return data[index];
    }

    function length() public returns (uint256) {
        return data.length;
    }
}

// ----
// length() -> 0
// get(uint256): 3 -> FAILURE, hex"4e487b71", 0x32
// enlarge(uint256): 4 -> 4
// length() -> 4
// set(uint256,uint256): 3, 4 -> true
// get(uint256): 3 -> 4
// length() -> 4
// set(uint256,uint256): 4, 8 -> FAILURE, hex"4e487b71", 0x32
// length() -> 4
