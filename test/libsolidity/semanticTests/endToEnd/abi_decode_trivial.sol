contract C {
    function f(bytes memory data) public pure returns(uint) {
        return abi.decode(data, (uint));
    }
}

// ----
// f(bytes): 0x20, 0x20, 33 -> 33
// f(bytes):"32, 32, 33" -> "33"
