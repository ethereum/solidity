// Used to cause ICE
contract C {
	function f() public {
		abi.decode("", (byte[999999999]));
	}
}
// ----
// TypeError 6118: (75-90): Type too large for memory.
