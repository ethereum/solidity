pragma experimental SMTChecker;

contract C
{
	function f() public pure {
		assembly {
		}
	}
}
// ----
// Warning 7737: (76-90): Inline assembly may cause SMTChecker to produce spurious warnings (false positives).
// Warning 7737: (76-90): Inline assembly may cause SMTChecker to produce spurious warnings (false positives).
