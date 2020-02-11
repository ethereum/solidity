contract c {
    fallback() external {
        data = msg.data;
    }

    function del() public returns(bool) {
        delete data;
        return true;
    }
    bytes data;
}

// ----
// ---: 7 -> bytes(
// ---:"7" -> ""
// del(): 7 -> true
// del():"7" -> "1"
