
		contract C {
			function f(uint a) public returns (uint b) {
				assembly {
					if gt(a, 1) { b := 2 }
				}
			}
		}
	
// ====
// optimize-yul: false
// ----
// f(uint256): 0x00 -> 0x00
// f(uint256): 0x01 -> 0x00
// f(uint256): 0x02 -> 0x02
// f(uint256): 0x03 -> 0x02

