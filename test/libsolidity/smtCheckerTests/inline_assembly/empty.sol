contract C
{
	function f() public pure {
		assembly {
		}
	}
}
// ====
// SMTEngine: all
// ----
// Warning 7737: (43-57): Inline assembly may cause SMTChecker to produce spurious warnings (false positives).
