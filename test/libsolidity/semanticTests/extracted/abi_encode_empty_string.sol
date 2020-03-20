// Tests that this will not end up using a "bytes0" type
// (which would assert)
contract C {
    function f() public pure returns (bytes memory, bytes memory) {
        return (abi.encode(""), abi.encodePacked(""));
    }
}
// ====
// ABIEncoderV1Only: true
// ----
// f() -> 0x40, 0xc0, 0x60, 0x20, 0x0, 0x0, 0x0
