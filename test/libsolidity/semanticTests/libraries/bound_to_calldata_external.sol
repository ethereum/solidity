library D {
    function f(bytes calldata _x) public pure returns (byte) {
        return _x[0];
    }
    function g(bytes memory _x) public pure returns (byte) {
        return _x[0];
    }
}

contract C {
    using D for bytes;
    function f(bytes calldata _x) public pure returns (byte, byte) {
        return (_x.f(), _x.g());
    }
}
// ====
// EVMVersion: >homestead
// ----
// library: D
// f(bytes): 0x20, 4, "abcd" -> 0x6100000000000000000000000000000000000000000000000000000000000000, 0x6100000000000000000000000000000000000000000000000000000000000000
