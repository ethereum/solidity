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
		assert(x < 11);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1180: Contract invariant(s) for :C:\n!(x >= 11)\nReentrancy property(ies) for :C:\n!(<errorCode> = 1)\n((!(x <= 10) || !(<errorCode> >= 3)) && (!(x <= 10) || !(x' >= 11)))\n<errorCode> = 0 -> no errors\n<errorCode> = 1 -> Overflow at ++x\n<errorCode> = 3 -> Assertion failed at assert(x < 11)\n
