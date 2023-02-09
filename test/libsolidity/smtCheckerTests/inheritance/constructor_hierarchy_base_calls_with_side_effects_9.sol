contract A {
	uint public x = 42;
	constructor(uint) {}

	function f() internal returns (uint) {
		x = x + 1;
		return x;
	}
}

contract C is A {
	constructor() A(f()) {
		assert(x == 42);
		assert(x == 0); // should fail
		assert(x == 1); // should fail
		assert(x > 2000); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (191-205): CHC: Assertion violation happens here.
// Warning 6328: (224-238): CHC: Assertion violation happens here.
// Warning 6328: (257-273): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
