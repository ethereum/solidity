contract C {

	function g() public pure returns (bytes memory) {
		return hex"ffff";
	}

	function f() public view {
		try this.g() returns (bytes memory b) {
			assert(b[0] == bytes1(uint8(255)) && b[1] == bytes1(uint8(255))); // should hold
			assert(b[0] == bytes1(uint8(0)) || b[1] == bytes1(uint8(0))); // should fail
		} catch {
		}
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (246-306): CHC: Assertion violation happens here.\nCounterexample:\n\nb = [255, 255]\n\nTransaction trace:\nC.constructor()\nC.f()\n    C.g() -- trusted external call
