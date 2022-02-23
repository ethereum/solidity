contract C {
    function f() public returns (uint8 x) {
        assembly {
            x := 0xffff
        }
        x >>= 8;
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f() -> 0x0
