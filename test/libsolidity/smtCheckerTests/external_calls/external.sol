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
// Info 1180: Reentrancy property(ies) for :C:\n!(<errorCode> = 1)\n<errorCode> = 0 -> no errors\n<errorCode> = 1 -> Overflow at ++x\n<errorCode> = 3 -> Assertion failed at assert(x < 10)\n
