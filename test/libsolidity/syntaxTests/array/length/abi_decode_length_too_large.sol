// Used to cause ICE
contract C {
	function f() public {
		abi.decode("", (byte[999999999]));
	}
}
// ----
// TypeError: (75-90): Type too large for memory.
