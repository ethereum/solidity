contract c {
    uint[4] data;

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
// length() -> 4
// length():"" -> "4"
// set(uint256,uint256): 3, 4 -> true
// set(uint256,uint256):"3, 4" -> "1"
// set(uint256,uint256): 4, 5 -> bytes(
// set(uint256,uint256):"4, 5" -> ""
// set(uint256,uint256): 400, 5 -> bytes(
// set(uint256,uint256):"400, 5" -> ""
// get(uint256): 3 -> 4
// get(uint256):"3" -> "4"
// get(uint256): 4 -> bytes(
// get(uint256):"4" -> ""
// get(uint256): 400 -> bytes(
// get(uint256):"400" -> ""
// length() -> 4
// length():"" -> "4"
