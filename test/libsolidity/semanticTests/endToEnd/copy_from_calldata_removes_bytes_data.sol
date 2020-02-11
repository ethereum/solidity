contract c {
    function set() public returns(bool) {
        data = msg.data;
        return true;
    }
    fallback() external {
        data = msg.data;
    }
    bytes data;
}

// ----
// set(): 1, 2, 3, 4, 5 -> true
// set():"1, 2, 3, 4, 5" -> "1"
