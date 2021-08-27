contract A {
}

contract C {
	function f() public pure {
		assert(bytes(type(C).name).length != 0);
		assert(type(A).creationCode.length != 0);
		assert(type(A).runtimeCode.length != 0);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 7507: (72-84): Assertion checker does not yet support this expression.
// Warning 7507: (109-129): Assertion checker does not yet support this expression.
// Warning 7507: (153-172): Assertion checker does not yet support this expression.
// Warning 6328: (59-98): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (102-142): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (146-185): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
