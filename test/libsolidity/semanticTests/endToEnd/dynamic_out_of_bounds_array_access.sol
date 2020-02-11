contract c {
    uint[] data;

    function enlarge(uint amount) public returns(uint) {
        while (data.length < amount)
            data.push();
        return data.length;
    }

    function set(uint index, uint value) public returns(bool) {
        data[index] = value;
        return true;
    }

    function get(uint index) public returns(uint) {
        return data[index];
    }

    function length() public returns(uint) {
        return data.length;
    }
}

// ====
// compileViaYul: also
// ----
// length() -> 0
// get(uint256): 3 -> FAILURE
// enlarge(uint256): 4 -> 4
// length() -> 4
// set(uint256,uint256): 3, 4 -> true
// get(uint256): 3 -> 4
// length() -> 4
// set(uint256,uint256): 4, 8 -> FAILURE
// length() -> 4
