contract C {
    bytes s;
    function f(bytes calldata c, string memory m) public view returns (bytes3, bytes8, bytes16, bytes32) {
        return (bytes3(c[0:3]), bytes8(s), bytes16(c), bytes32(bytes(m)));
    }
}
// ----