contract C {
	/// @custom:smtchecker abstract-function-nondet
	function f(uint x) internal pure returns (uint) {
		return x;
	}
	function g(uint y) public pure {
		uint z = f(y);
		// Generally holds, but here it doesn't because function
		// `f` has been abstracted by nondeterministic values.
		assert(z == y);
	}
}
// ====
// SMTEngine: chc
// SMTIgnoreCex: yes
// ----
// Warning 6328: (297-311): CHC: Assertion violation happens here.
