pragma experimental SMTChecker;

contract C {
	struct S {
		uint x;
		uint[] a;
	}
	S s;
	function p() public { s.a.push(); }
	function f(uint _x) public {
		require(s.a.length >= 2);
		s.x = _x;
		s.a[0] = _x;
		assert(s.a[1] == s.a[0]);
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 6328: (213-237): CHC: Assertion violation happens here.
