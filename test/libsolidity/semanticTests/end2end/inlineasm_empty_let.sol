
		contract C {
			function f() public pure returns (uint a, uint b) {
				assembly {
					let x
					let y, z
					a := x
					b := z
				}
			}
		}
	
// ====
// optimize-yul: false
// ----
// f() -> 0x00, 0x00

