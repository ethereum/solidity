pragma experimental SMTChecker;

contract C
{
	function f(string memory s1, string memory s2) public pure {
		assert(bytes(s1).length == bytes(s2).length);
	}
}
// ----
// Warning: (117-133): Assertion checker does not yet support this expression.
// Warning: (137-153): Assertion checker does not yet support this expression.
// Warning: (110-154): Assertion violation happens here
