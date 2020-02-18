
		contract C {
			function f(uint a) public returns (uint b) {
				assembly {
					switch a
					case 1 { b := 8 }
					case 2 { b := 9 }
					default { b := 2 }
				}
			}
		}
	
// ====
// optimize-yul: false
// ----
// f(uint256): 0x00 -> 0x02
// f(uint256): 0x01 -> 0x08
// f(uint256): 0x02 -> 0x09
// f(uint256): 0x03 -> 0x02

