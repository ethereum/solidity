contract C {
	/// @custom:smtchecker abstract-function
	function f(uint x) internal pure returns (uint) {
		return x;
	}
}
// ====
// SMTEngine: chc
// ----
// Warning 3130: (56-120): Unknown option for "custom:smtchecker": "abstract-function"
// Warning 3130: (56-120): Unknown option for "custom:smtchecker": "abstract-function"
