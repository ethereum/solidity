contract C {
    bytes data;

    function f(bytes memory _data) public returns(uint, bytes memory) {
        data = _data;
        return abi.decode(data, (uint, bytes));
    }
}

// ----
// f(bytes): 32, 128, 33, 64, 7, "abcdefg" -> 33, 64, 7, "abcdefg"
