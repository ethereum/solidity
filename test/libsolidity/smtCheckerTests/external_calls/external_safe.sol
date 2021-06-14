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
// Warning 0: (57-187): Contract invariants for :C:\n!(<errorCode> = 1)\n!(x >= 11)\n((!(x <= 10) || !(<errorCode> >= 2)) && (!(x <= 10) || !(x' >= 11)))\n
