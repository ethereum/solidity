
		contract C {
			function f(uint x) public returns (uint a) {
				assembly {
					a := byte(x, 31)
				}
			}
			function g(uint x) public returns (uint a) {
				assembly {
					a := byte(31, x)
				}
			}
		}
	
// ====
// optimize-yul: false
// ----
// f(uint256): 0x02 -> 0x00
// g(uint256): 0x02 -> 0x02

