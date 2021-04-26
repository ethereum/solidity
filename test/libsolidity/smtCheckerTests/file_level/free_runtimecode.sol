contract C {
	uint public x = 2;
}

function test() pure returns (bool) {
	return type(C).runtimeCode.length > 20;
}

contract D {
	function f() public pure {
		assert(test()); // should hold but SMTChecker doesn't know that
	}
}
// ====
// SMTEngine: all
// ----
// Warning 7507: (82-101): Assertion checker does not yet support this expression.
// Warning 7507: (82-101): Assertion checker does not yet support this expression.
// Warning 6328: (161-175): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nD.constructor()\nD.f()\n    test() -- internal call
// Warning 7507: (82-101): Assertion checker does not yet support this expression.
