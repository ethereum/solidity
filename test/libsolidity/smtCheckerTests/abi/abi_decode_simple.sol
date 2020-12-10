pragma experimental SMTChecker;
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
// SMTIgnoreCex: yes
// ----
// Warning 6328: (241-255): CHC: Assertion violation happens here.
// Warning 6328: (292-306): CHC: Assertion violation happens here.
// Warning 6328: (391-405): CHC: Assertion violation happens here.
// Warning 6328: (424-438): CHC: Assertion violation happens here.
// Warning 6328: (457-466): CHC: Assertion violation happens here.
// Warning 6328: (537-551): CHC: Assertion violation happens here.
// Warning 6328: (570-584): CHC: Assertion violation happens here.
