pragma experimental SMTChecker;
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
// ----
// Warning 6328: (278-338): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()\n    C.g() -- trusted external call
