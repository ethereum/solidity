contract C
{
	function f(uint[2] memory a, uint[2] memory b, uint[2] memory c) public pure {
		require(a.length > 0);
		require(b.length > 0);
		require(c.length > 0);
		require(c[0] == 42);
		require(a[0] == 2);
		b[0] = 1;
		// Should fail since b == c is possible.
		// Removed because current Spacer seg faults in cex generation.
		//assert(c[0] == 42);
		// Should fail since b == a is possible.
		// Access is safe but oob is reported due of aliasing.
		assert(a[0] == 2);
		assert(b[0] == 1);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6368: (467-471): CHC: Out of bounds access happens here.
// Warning 6328: (460-477): CHC: Assertion violation happens here.
