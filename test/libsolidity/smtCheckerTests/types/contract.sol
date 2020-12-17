pragma experimental SMTChecker;

contract C
{
	function f(C c, C d) public pure {
		assert(c == d);
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 6328: (84-98): CHC: Assertion violation happens here.
