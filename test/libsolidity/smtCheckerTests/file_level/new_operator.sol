contract C {
	uint public x = 2;
}

function test() returns (uint) {
	return (new C()).x();
}

contract D {
	function f() public {
		assert(test() == 2); // should hold but the SMTChecker doesn't support `new`
	}
}
// ====
// SMTEngine: all
// ----
// Warning 4588: (78-85): Assertion checker does not yet implement this type of function call.
// Warning 6328: (133-152): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nD.constructor()\nD.f()\n    test() -- internal call\n        (new C()).x() -- untrusted external call
