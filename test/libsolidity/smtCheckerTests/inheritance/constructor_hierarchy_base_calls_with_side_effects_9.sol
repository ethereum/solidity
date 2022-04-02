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
// Warning 6328: (191-205='assert(x == 0)'): CHC: Assertion violation happens here.\nCounterexample:\nx = 42\n\nTransaction trace:\nC.constructor()
// Warning 6328: (224-238='assert(x == 1)'): CHC: Assertion violation happens here.\nCounterexample:\nx = 42\n\nTransaction trace:\nC.constructor()
// Warning 6328: (257-273='assert(x > 2000)'): CHC: Assertion violation happens here.\nCounterexample:\nx = 42\n\nTransaction trace:\nC.constructor()
