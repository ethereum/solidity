library D {
    function f(bytes calldata _x) internal pure returns (bytes calldata) {
        return _x;
    }
    function g(bytes calldata _x) internal pure returns (bytes memory) {
        return _x;
    }
}

contract C {
    using D for bytes;
    function f(bytes calldata _x) public pure returns (bytes1, bytes1) {
        return (_x.f()[0], _x.g()[0]);
    }
}
// ----
// f(bytes): 0x20, 4, "abcd" -> 0x6100000000000000000000000000000000000000000000000000000000000000, 0x6100000000000000000000000000000000000000000000000000000000000000
