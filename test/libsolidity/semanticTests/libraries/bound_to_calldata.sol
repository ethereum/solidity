library D {
    function f(bytes calldata _x) internal pure returns (bytes1) {
        return _x[0];
    }
    function g(bytes memory _x) internal pure returns (bytes1) {
        return _x[0];
    }
}

contract C {
    using D for bytes;
    function f(bytes calldata _x) public pure returns (bytes1, bytes1) {
        return (_x.f(), _x.g());
    }
}
// ====
// compileViaYul: also
// ----
// f(bytes): 0x20, 4, "abcd" -> 0x6100000000000000000000000000000000000000000000000000000000000000, 0x6100000000000000000000000000000000000000000000000000000000000000
