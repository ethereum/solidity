contract c {
    uint256[4] data;

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

// ====
// compileViaYul: also
// ----
// length() -> 4
// set(uint256,uint256): 3, 4 -> true
// set(uint256,uint256): 4, 5 -> FAILURE
// set(uint256,uint256): 400, 5 -> FAILURE
// get(uint256): 3 -> 4
// get(uint256): 4 -> FAILURE
// get(uint256): 400 -> FAILURE
// length() -> 4
