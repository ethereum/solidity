pragma experimental SMTChecker;

contract C
{
	function f(bytes memory b1, bytes memory b2) public pure {
		b1 = b2;
		require(b1.length > 2 && b2.length > 2);
		assert(b1[1] == b2[2]);
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 6328: (162-184): CHC: Assertion violation happens here.
