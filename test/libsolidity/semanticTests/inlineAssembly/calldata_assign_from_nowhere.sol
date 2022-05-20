contract C {
    function f() public pure returns (bytes calldata x) {
        assembly { x.offset := 0 x.length := 4 }
    }
}
// ----
// f() -> 0x20, 4, 0x26121ff000000000000000000000000000000000000000000000000000000000
