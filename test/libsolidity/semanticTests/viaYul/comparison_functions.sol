contract C {
	function internal1() internal pure returns (bool) {
		return true;
	}
	function internal2() internal pure returns (bool) {
		return true;
	}

	function equal() public pure returns (bool same, bool diff) {
		same = internal1 == internal1;
		diff = internal1 == internal2;
	}

	function unequal() public pure returns (bool same, bool diff) {
		same = internal1 != internal1;
		diff = internal1 != internal2;
	}
}
// ====
// compileViaYul: true
// ----
// equal() -> true, false
// unequal() -> false, true
