contract c {
    function set() public returns(bool) {
        data = msg.data;
        return true;
    }

    function getLength() public returns(uint) {
        return data.length;
    }
    bytes data;
}

// ----
// getLength() -> 0
// getLength():"" -> "0"
// set(): 1, 2 -> true
// set():"1, 2" -> "1"
// getLength() -> 4+32+32
// getLength():"" -> "68"
