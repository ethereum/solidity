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
// Warning 6328: (247-267): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
