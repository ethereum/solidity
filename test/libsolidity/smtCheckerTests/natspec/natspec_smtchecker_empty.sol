contract C {
	/// @custom:smtchecker
	function f(uint x) internal pure returns (uint) {
		return x;
	}
}
// ====
// SMTEngine: chc
// ----
// Warning 3130: (38-102): Unknown option for "custom:smtchecker": ""
// Warning 3130: (38-102): Unknown option for "custom:smtchecker": ""
