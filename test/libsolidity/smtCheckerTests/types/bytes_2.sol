contract C
{
	function f(bytes memory b1, bytes memory b2) public pure {
		require(b2.length > 2);
		b1 = b2;
		// Knowledge about b2 is lost because of potential aliasing, so we re-add the length constraint.
		require(b2.length > 2);
		assert(b1[1] == b2[1]);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (237-259): CHC: Assertion violation happens here.
