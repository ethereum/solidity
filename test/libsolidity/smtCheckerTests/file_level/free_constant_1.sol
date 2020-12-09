pragma experimental SMTChecker;
uint constant A = 42;
contract C {
	function f(uint x) public pure returns (uint) {
		return x + A;
	}
}
// ----
// Warning 8195: (32-52): Model checker analysis was not possible because file level constants are not supported.
// Warning 8195: (32-52): Model checker analysis was not possible because file level constants are not supported.
