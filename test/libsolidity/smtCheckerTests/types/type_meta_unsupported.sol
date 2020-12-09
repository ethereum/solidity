pragma experimental SMTChecker;

contract A {
}

contract C {
	function f() public pure {
		assert(bytes(type(C).name).length != 0);
		assert(type(A).creationCode.length != 0);
		assert(type(A).runtimeCode.length != 0);
	}
}
// ----
// Warning 7507: (105-117): Assertion checker does not yet support this expression.
// Warning 7507: (142-162): Assertion checker does not yet support this expression.
// Warning 7507: (186-205): Assertion checker does not yet support this expression.
// Warning 6328: (92-131): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (135-175): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (179-218): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 7507: (105-117): Assertion checker does not yet support this expression.
// Warning 7507: (142-162): Assertion checker does not yet support this expression.
// Warning 7507: (186-205): Assertion checker does not yet support this expression.
