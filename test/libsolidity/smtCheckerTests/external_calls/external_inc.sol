abstract contract D {
	function d() external virtual;
}

contract C {
	uint x;
	D d;

	function inc() public {
		++x;
	}

	function f() public {
		d.d();
		assert(x < 10);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 4984: (113-116): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 4984: (113-116): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 6328: (156-170): CHC: Assertion violation happens here.
// Warning 2661: (113-116): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
