contract C {

	function g() public pure returns (bytes2) {
		return hex"ffff";
	}

	function f() public view {
		try this.g() returns (bytes2 b) {
			assert(uint8(b[0]) == 255 && uint8(b[1]) == 255); // should hold
			assert(uint8(b[0]) == 0 || uint8(b[1]) == 0); // should fail
		} catch {
		}
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (218-262): CHC: Assertion violation happens here.
// Info 1391: CHC: 5 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
