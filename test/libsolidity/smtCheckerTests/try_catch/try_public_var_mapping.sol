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
// Warning 6328: (280-300): CHC: Assertion violation happens here.
// Info 1391: CHC: 6 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
