contract C {
    function f() public pure returns (uint8 x) {
        uint8 y = uint8(2)**uint8(8);
        return 0**y;
    }
}

// ----
// f() -> 0x1
