contract C {
	mapping (uint => uint[]) public m;

	constructor() {
		m[0].push();
		m[0].push();
		m[0][1] = 42;
	}

	function f() public view {
		try this.m(0,1) returns (uint y) {
			assert(y == m[0][1]); // should hold
		}
		catch {
			assert(m[0][1] == 42); // should hold
			assert(m[0][1] == 1); // should fail
		}
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 6328: (280-300): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Info 1180: Contract invariant(s) for :C:\n!(m[0].length <= 1)\n(!(m[0][1] <= 41) && !(m[0][1] >= 43))\n
