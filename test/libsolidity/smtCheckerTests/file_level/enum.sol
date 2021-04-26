enum E {
	READ,
	WRITE
}

function allocate(bool b) pure returns (E) {
	if (b) return E.READ;
	return E.WRITE;
}

contract C {
	function f() public pure {
		E e1 = allocate(true);
		assert(e1 == E.READ); // should hold
		E e2 = allocate(false);
		assert(e2 == E.READ); // should fail
		assert(allocate(false) == E.WRITE); // should hold
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (247-267): CHC: Assertion violation happens here.\nCounterexample:\n\ne1 = 0\ne2 = 1\n\nTransaction trace:\nC.constructor()\nC.f()\n    allocate(true) -- internal call\n    allocate(false) -- internal call
