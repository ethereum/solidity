contract C
{
	uint[] s;
	function f() public {
		uint[3] memory a = [uint(1), 2, 3];
		uint[4] memory b = [uint(1), 2, 4, 3];
		uint[4] memory c = b;
		assert(c.length == b.length);
		s = a;
		assert(s.length == a.length);

		assert(s.length == c.length); // fails
		assert(s[0] == c[0]);
		assert(s[1] == c[1]);
		assert(s[2] == c[2]); // fails
		assert(s[2] == c[3]);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (226-254): CHC: Assertion violation happens here.
// Warning 6328: (315-335): CHC: Assertion violation happens here.
// Info 1391: CHC: 13 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
