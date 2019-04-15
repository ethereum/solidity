pragma experimental SMTChecker;

contract C
{
	function f() public pure {
		assembly {
		}
	}
}
// ----
// Warning: (76-90): Assertion checker does not support inline assembly.
