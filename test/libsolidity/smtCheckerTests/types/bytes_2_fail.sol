pragma experimental SMTChecker;

contract C
{
	function f(bytes memory b1, bytes memory b2) public pure {
		b1 = b2;
		assert(b1[1] == b2[2]);
	}
}
// ----
// Warning 4661: (119-141): Assertion violation happens here
