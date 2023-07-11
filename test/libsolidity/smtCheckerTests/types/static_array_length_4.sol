contract C {
	uint[2] a;
	uint x = f();
	constructor() {
		assert(a.length == 2); // should hold
		assert(x == 2); // should hold
		assert(a.length < 2); // should fail
		assert(a.length > 2); // should fail
	}
	function f() internal view returns (uint) {
		assert(a.length == 2); // should hold
		assert(a.length < 2); // should fail
		assert(a.length > 2); // should fail
		return a.length;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (132-152): CHC: Assertion violation happens here.
// Warning 6328: (171-191): CHC: Assertion violation happens here.
// Warning 6328: (298-318): CHC: Assertion violation happens here.
// Warning 6328: (337-357): CHC: Assertion violation happens here.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
