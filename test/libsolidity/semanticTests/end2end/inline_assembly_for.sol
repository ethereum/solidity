
		contract C {
			function f(uint a) public returns (uint b) {
				assembly {
					function fac(n) -> nf {
						nf := 1
						for { let i := n } gt(i, 0) { i := sub(i, 1) } {
							nf := mul(nf, i)
						}
					}
					b := fac(a)
				}
			}
		}
	
// ====
// optimize-yul: false
// ----
// f(uint256): 0x00 -> 0x01
// f(uint256): 0x01 -> 0x01
// f(uint256): 0x02 -> 0x02
// f(uint256): 0x03 -> 0x06
// f(uint256): 0x04 -> 0x18

