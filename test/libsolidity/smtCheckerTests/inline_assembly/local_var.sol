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
// Warning: (97-130): Assertion checker does not support inline assembly.
