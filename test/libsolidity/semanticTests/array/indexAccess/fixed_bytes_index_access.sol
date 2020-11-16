contract C {
    bytes16[] public data;

    function f(bytes32 x) public returns (bytes1) {
        return x[2];
    }

    function g(bytes32 x) public returns (uint256) {
        data = [x[0], x[1], x[2]];
        data[0] = "12345";
        return uint256(uint8(data[0][4]));
    }
}
// ====
// compileViaYul: also
// ----
// f(bytes32): "789" -> "9"
// g(bytes32): "789" -> 0x35
// data(uint256): 0x01 -> "8"
