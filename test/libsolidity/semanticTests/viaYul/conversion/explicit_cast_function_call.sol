contract C {
    function f(bytes32 b) public pure returns (bytes32 x) {
        x = b;
    }
    function g() public pure returns (bytes32 x) {
        x = f(bytes4(uint32(0x12345678)));
    }
}
// ====
// compileViaYul: true
// ----
// g() -> 0x1234567800000000000000000000000000000000000000000000000000000000
