contract C
{
	uint[] s;
	function f() public {
		uint[3] memory a = [uint(1), 2, 3];
		s = a;
		assert(s.length == a.length);
		assert(s[0] == a[0]);
		assert(s[1] == a[1]);
		assert(s[2] != a[2]); // fails
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (176-196): CHC: Assertion violation happens here.
// Info 1391: CHC: 9 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
