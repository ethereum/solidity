pragma experimental SMTChecker;

contract C
{
	function f(uint x) public pure returns (uint) {
		assembly {
			x := 2
		}
		return x;
	}
}
// ----
// Warning 7737: (97-121): Inline assembly may cause SMTChecker to produce spurious warnings (false positives).
// Warning 7737: (97-121): Inline assembly may cause SMTChecker to produce spurious warnings (false positives).
