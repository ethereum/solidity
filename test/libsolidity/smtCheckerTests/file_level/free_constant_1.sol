uint constant A = 42;
contract C {
	function f(uint x) public pure returns (uint) {
		return x + A;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 8195: (0-20): Model checker analysis was not possible because file level constants are not supported.
// Warning 8195: (0-20): Model checker analysis was not possible because file level constants are not supported.
