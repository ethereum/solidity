contract C {
	function f(bytes memory data) public pure {
		bytes32 k = keccak256(data);
		bytes32 s = sha256(data);
		bytes32 r = ripemd160(data);
		assert(k == s);
		assert(s == r);
		assert(r == k);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (150-164): CHC: Assertion violation happens here.
// Warning 6328: (168-182): CHC: Assertion violation happens here.
// Warning 6328: (186-200): CHC: Assertion violation happens here.
