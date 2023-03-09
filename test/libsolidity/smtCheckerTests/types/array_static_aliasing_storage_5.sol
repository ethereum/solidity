contract C
{
	uint[2] b1;
	uint[2] b2;
	function f(uint[2] storage a, uint[2] memory c) internal {
		// Accesses are safe but oob is reported because of aliasing.
		c[0] = 42;
		a[0] = 2;
		b1[0] = 1;
		// Erasing knowledge about storage variables should not
		// erase knowledge about memory references.
		assert(c[0] == 42);
		// Fails because b1 == a is possible.
		// Disabled because Spacer seg faults.
		//assert(a[0] == 2);
		assert(b1[0] == 1);
	}
	function g(bool x, uint[2] memory c) public {
		if (x) f(b1, c);
		else f(b2, c);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Info 1391: CHC: 7 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
