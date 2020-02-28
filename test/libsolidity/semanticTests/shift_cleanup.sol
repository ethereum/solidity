contract C {
    function f() public returns (uint16 x) {
        x = 0xffff;
        x += 32;
        x <<= 8;
        x >>= 16;
    }
}

// ----
// f() -> 0x0
