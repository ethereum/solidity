
		contract c {
			function f(uint a) public returns (uint) { return a; }
			function test(uint a, uint b) external returns (uint r_a, uint r_b) {
				r_a = f(a + 7);
				r_b = b;
			}
		}
	
// ====
// optimize-yul: false
// ----
// test(uint256,uint256): 0x2, 0x3 -> 0x9, 0x3

