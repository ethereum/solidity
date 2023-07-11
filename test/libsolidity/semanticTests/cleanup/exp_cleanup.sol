contract C {
    function f() public pure returns (uint x) {
        unchecked {
            uint8 y = uint8(2)**uint8(8);
            return 0**y;
        }
    }
}
// ----
// f() -> 0x1
