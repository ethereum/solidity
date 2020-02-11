contract C {
    function f(bytes memory data) public pure returns(uint) {
        return abi.decode(data, (uint));
    }
}
// ----
// f(bytes): 32, 32, 33 -> 33
