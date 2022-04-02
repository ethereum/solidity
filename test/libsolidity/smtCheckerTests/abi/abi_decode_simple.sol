contract C {
	function abiDecodeSimple(bytes memory b1, bytes memory b2) public pure {
		(uint x, uint y) = abi.decode(b1, (uint, uint));
		(uint z, uint w) = abi.decode(b1, (uint, uint));
		assert(x == z);
		assert(x == y); // should fail
		assert(y == w);
		assert(z == w); // should fail

		(uint a, uint b, bool c) = abi.decode(b1, (uint, uint, bool));
		assert(a == x); // should fail
		assert(b == y); // should fail
		assert(c); // should fail

		(uint k, uint l) = abi.decode(b2, (uint, uint));
		assert(k == x); // should fail
		assert(l == y); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 1218: (505-519='assert(k == x)'): CHC: Error trying to invoke SMT solver.
// Warning 1218: (538-552='assert(l == y)'): CHC: Error trying to invoke SMT solver.
// Warning 6328: (209-223='assert(x == y)'): CHC: Assertion violation happens here.
// Warning 6328: (260-274='assert(z == w)'): CHC: Assertion violation happens here.
// Warning 6328: (359-373='assert(a == x)'): CHC: Assertion violation happens here.
// Warning 6328: (392-406='assert(b == y)'): CHC: Assertion violation happens here.
// Warning 6328: (425-434='assert(c)'): CHC: Assertion violation happens here.
// Warning 6328: (505-519='assert(k == x)'): CHC: Assertion violation might happen here.
// Warning 6328: (538-552='assert(l == y)'): CHC: Assertion violation might happen here.
// Warning 4661: (505-519='assert(k == x)'): BMC: Assertion violation happens here.
// Warning 4661: (538-552='assert(l == y)'): BMC: Assertion violation happens here.
