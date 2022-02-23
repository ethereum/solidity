// Used to cause ICE
contract C {
	function f() public {
		abi.decode("", (bytes1[999999999]));
	}
}
// ----
// TypeError 6118: (75-92): Type too large for memory.
