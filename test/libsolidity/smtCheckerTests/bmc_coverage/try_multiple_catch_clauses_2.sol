contract C {

	function g() public pure {}

	function f() public view {
		uint x = 0;
		bool success = false;
		try this.g() {
			success = true;
			x = 1;
		} catch Error (string memory /*reason*/) {
			x = 2;
		} catch (bytes memory /*reason*/) {
			x = 3;
		}
		assert(x > 0 && x < 4); // should hold
		assert(x == 0); // should fail
	}
}
// ====
// SMTEngine: bmc
// ----
// Warning 4661: (306-320): BMC: Assertion violation happens here.
