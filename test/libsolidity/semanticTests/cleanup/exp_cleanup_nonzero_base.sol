contract C {
    function f() public pure returns (uint8 x) {
        unchecked {
			uint16 x = 0x166;
            return uint8(x)**uint8(uint8(2)**uint8(8));
        }
    }
}

// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f() -> 0x1
