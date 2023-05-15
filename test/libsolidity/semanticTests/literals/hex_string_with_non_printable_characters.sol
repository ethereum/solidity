contract C {
    function f() public pure returns (bytes32 result) {
        assembly {
            result := hex"000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f"
        }
    }
}
// ----
// f() -> 0x000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f
