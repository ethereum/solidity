contract C {
    function f() public pure returns (uint8 x) {
        unchecked {
            return uint8(0)**uint8(uint8(2)**uint8(8));
        }
    }
}

// ====
// compileToEwasm: also
// ----
// f() -> 0x1
