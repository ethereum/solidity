contract C {
	// If these two functions are identical, the optimiser
	// on the old codegen path can deduplicate them, and breaking the test.
	function internal1() internal pure returns (bool) {
		return true;
	}
	function internal2() internal pure returns (bool) {
		return false;
	}

	function equal() public pure returns (bool same, bool diff, bool inv) {
		function() internal pure returns (bool) invalid;
		delete invalid;
		same = internal1 == internal1;
		diff = internal1 == internal2;
		inv  = internal1 == invalid;
	}

	function unequal() public pure returns (bool same, bool diff, bool inv) {
		function() internal pure returns (bool) invalid;
		delete invalid;
		same = internal1 != internal1;
		diff = internal1 != internal2;
		inv  = internal1 != invalid;
	}
}
// ----
// equal() -> true, false, false
// unequal() -> false, true, true
