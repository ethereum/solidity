contract C {
    function f() public pure returns (bytes memory) {
        return abi.encodePacked(
            ufixed16x3(1.23),
            fixed64x4(-1.234)
        );
    }
    function g() public pure returns (bytes memory) {
        return abi.encode(
            1.23,
            fixed64x4(-1.234)
        );
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 0x20, 10, 0x4ceffffffffffffcfcc00000000000000000000000000000000000000000000
// g() -> 0x20, 0x40, 0x7b, -12340
