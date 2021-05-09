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
// SMTEngine: all
// ----
// Warning 6328: (306-320): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 3\nsuccess = false\n\nTransaction trace:\nC.constructor()\nC.f()
