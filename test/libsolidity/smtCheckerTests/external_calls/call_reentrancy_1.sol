contract C {
	uint x;
	function s(uint _x) public {
		x = _x;
	}
	function f(address a) public {
		(bool s, bytes memory data) = a.call("");
		assert(x == 0); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 2519: (100-106): This declaration shadows an existing declaration.
// Warning 2072: (100-106): Unused local variable.
// Warning 2072: (108-125): Unused local variable.
// Warning 6328: (143-157): CHC: Assertion violation happens here.
