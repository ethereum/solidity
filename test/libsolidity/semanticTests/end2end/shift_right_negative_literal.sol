
			contract C {
				function f1() public pure returns (bool) {
					return (-4266 >> 0) == -4266;
				}
				function f2() public pure returns (bool) {
					return (-4266 >> 1) == -2133;
				}
				function f3() public pure returns (bool) {
					return (-4266 >> 4) == -267;
				}
				function f4() public pure returns (bool) {
					return (-4266 >> 8) == -17;
				}
				function f5() public pure returns (bool) {
					return (-4266 >> 16) == -1;
				}
				function f6() public pure returns (bool) {
					return (-4266 >> 17) == -1;
				}
				function g1() public pure returns (bool) {
					return (-4267 >> 0) == -4267;
				}
				function g2() public pure returns (bool) {
					return (-4267 >> 1) == -2134;
				}
				function g3() public pure returns (bool) {
					return (-4267 >> 4) == -267;
				}
				function g4() public pure returns (bool) {
					return (-4267 >> 8) == -17;
				}
				function g5() public pure returns (bool) {
					return (-4267 >> 16) == -1;
				}
				function g6() public pure returns (bool) {
					return (-4267 >> 17) == -1;
				}
			}
		
// ====
// optimize-yul: false
// ----
// f1() -> 0x1
// f2() -> 0x1
// f3() -> 0x1
// f4() -> 0x1
// f5() -> 0x1
// f6() -> 0x1
// g1() -> 0x1
// g2() -> 0x1
// g3() -> 0x1
// g4() -> 0x1
// g5() -> 0x1
// g6() -> 0x1

