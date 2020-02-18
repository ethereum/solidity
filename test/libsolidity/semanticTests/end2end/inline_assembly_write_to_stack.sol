
		contract C {
			function f() public returns (uint r, bytes32 r2) {
				assembly { r := 7 r2 := "abcdef" }
			}
		}
	
// ====
// optimize-yul: false
// ----
// f() -> 0x07, "abcdef"

