abstract contract D {
	function d() external virtual;
}

contract C {
	uint x;
	D d;
	function f() public {
		if (x < 10)
			++x;
	}
	function g() public {
		d.d();
		assert(x < 10);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (167-181): CHC: Assertion violation happens here.
// Warning 0: (57-187): Contract invariants for :C:\n!(<errorCode> = 1)\n
