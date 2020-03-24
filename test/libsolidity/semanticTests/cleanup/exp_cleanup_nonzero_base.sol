contract C {
    function f() public pure returns (uint8 x) {
        return uint8(0x166)**uint8(uint8(2)**uint8(8));
    }
}

// ----
// f() -> 0x1
