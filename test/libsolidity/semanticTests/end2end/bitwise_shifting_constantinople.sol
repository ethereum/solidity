
		contract C {
			function shl(uint a, uint b) public returns (uint c) {
				assembly {
					c := shl(b, a)
				}
			}
			function shr(uint a, uint b) public returns (uint c) {
				assembly {
					c := shr(b, a)
				}
			}
			function sar(uint a, uint b) public returns (uint c) {
				assembly {
					c := sar(b, a)
				}
			}
		}
	
// ====
// optimize-yul: false
// ----
// shl(uint256,uint256): 0x01, 0x02 -> 0x04
// shl(uint256,uint256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x01 -> 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe
// shl(uint256,uint256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x100 -> 0x00
// shr(uint256,uint256): 0x03, 0x01 -> 0x01
// shr(uint256,uint256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x01 -> 0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// shr(uint256,uint256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xff -> 0x01
// shr(uint256,uint256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x100 -> 0x00
// sar(uint256,uint256): 0x03, 0x01 -> 0x01
// sar(uint256,uint256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x01 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// sar(uint256,uint256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xff -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// sar(uint256,uint256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x100 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff

