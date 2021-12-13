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
// Warning 1218: (150-164): CHC: Error trying to invoke SMT solver.
// Warning 1218: (168-182): CHC: Error trying to invoke SMT solver.
// Warning 1218: (186-200): CHC: Error trying to invoke SMT solver.
// Warning 6328: (150-164): CHC: Assertion violation might happen here.
// Warning 6328: (168-182): CHC: Assertion violation might happen here.
// Warning 6328: (186-200): CHC: Assertion violation might happen here.
// Warning 4661: (150-164): BMC: Assertion violation happens here.
// Warning 4661: (168-182): BMC: Assertion violation happens here.
// Warning 4661: (186-200): BMC: Assertion violation happens here.
