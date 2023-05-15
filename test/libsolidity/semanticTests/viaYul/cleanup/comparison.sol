contract C {
	function eq() public pure returns (bool) {
		uint8 x = 1; uint8 y;
		assembly { y := 0x0101 }
		return (x == y);
	}
	function neq() public pure returns (bool) {
		uint8 x = 1; uint8 y;
		assembly { y := 0x0101 }
		return (x != y);
	}
	function geq() public pure returns (bool) {
		uint8 x = 1; uint8 y;
		assembly { y := 0x0101 }
		return (x >= y);
	}
	function leq() public pure returns (bool) {
		uint8 x = 2; uint8 y;
		assembly { y := 0x0101 }
		return (x <= y);
	}
	function gt() public pure returns (bool) {
		uint8 x = 2; uint8 y;
		assembly { y := 0x0101 }
		return (x > y);
	}
	function lt() public pure returns (bool) {
		uint8 x = 1; uint8 y;
		assembly { y := 0x0101 }
		return (x < y);
	}
}
// ----
// eq() -> true
// neq() -> false
// geq() -> true
// leq() -> false
// gt() -> true
// lt() -> false
