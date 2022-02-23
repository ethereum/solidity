contract C {

	function f() public view {
		assert(msg.sender >= address(0)); // should hold
		assert(msg.sender <= address(2**160-1)); // should hold
	}
}
// ====
// SMTEngine: all
// ----
