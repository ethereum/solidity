
		contract C {
			function f(uint a) public returns (uint b) {
				assembly {
					function fac(n) -> nf {
						switch n
						case 0 { nf := 1 }
						case 1 { nf := 1 }
						default { nf := mul(n, fac(sub(n, 1))) }
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

