contract C {
	int public x;

	function f() public view {
		try this.x() returns (int v) {
			assert(x == v); // should hold
		} catch {
			assert(false); // this fails, because we over-approximate every external call in the way that it can both succeed and fail
		}
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (139-152): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\nTransaction trace:\nC.constructor()\nState: x = 0\nC.f()
