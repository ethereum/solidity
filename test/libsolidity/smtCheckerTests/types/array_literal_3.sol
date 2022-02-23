contract C
{
	function f() public pure {
		(uint[3] memory a, uint[3] memory b) = ([uint(1), 2, 3], [uint(1), 2, 4]);
		assert(a[0] == b[0]);
		assert(a[1] == b[1]);
		assert(a[2] == b[2]); // fails
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (168-188): CHC: Assertion violation happens here.
