contract C {

	function g() public pure returns (bytes memory) {
		return hex"ffff";
	}

	function f() public view {
		try this.g() returns (bytes memory b) {
			assert(b[0] == bytes1(uint8(255)) && b[1] == bytes1(uint8(255))); // should hold
			// Disabled because of Spacer seg fault
			//assert(b[0] == bytes1(uint8(0)) || b[1] == bytes1(uint8(0))); // should fail
		} catch {
		}
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
